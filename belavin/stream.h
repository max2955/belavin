#ifndef BELAVIN_STREAM
#define BELAVIN_STREAM

#include "configuration.h"
#include "queue.h"

#define ACTIVE_STREAM_REF_COUNT 2

typedef struct {
	belavin_queue_t input_queue;
	belavin_queue_t output_queue;
	GMutex mutex;
	int ref_count;
} belavin_stream_t;

void stream_inc_ref_count(belavin_stream_t*);
void stream_dec_ref_count(belavin_stream_t*);
int stream_get_ref_count(belavin_stream_t*);

#endif // BELAVIN_STREAM
