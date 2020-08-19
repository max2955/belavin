#ifndef BELAVIN_QUEUE
#define BELAVIN_QUEUE 1

#include <glib.h>

#define MAX_QUEUE_DEPTH 8
#define QUEUE_TYPE_INPUT 1
#define QUEUE_TYPE_OUTPUT 2
#define QUEUE_ENQUE_SIGNAL 1
#define QUEUE_ENQUE_NOSIGNAL 2

typedef struct {
	gpointer *items[MAX_QUEUE_DEPTH];
	int item_lens[MAX_QUEUE_DEPTH];
	gint front, rear, size, capacity;
	GMutex mutex;
	GCond data_arrived;
	int type;
} belavin_queue_t;

int queue_init(belavin_queue_t *queue, int type);
int queue_wait(belavin_queue_t *queue, int secs);
gint queue_clear(belavin_queue_t *queue);
int queue_enqueue(belavin_queue_t *queue, gpointer in, int inlen, int signal);
int queue_dequeue(belavin_queue_t *queue, gpointer out, int *outlen);
void queue_dump(belavin_queue_t *queue);
void queue_reset(belavin_queue_t *queue);
int queue_get_size(belavin_queue_t *queue);

#endif // BELAVIN_QUEUE
