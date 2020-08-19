#include <glib.h>

#include "queue.h"
#include "netpdu.h"

void queue_reset(belavin_queue_t *queue) {
	queue->capacity = MAX_QUEUE_DEPTH;
	queue->front = 0;
	queue->size = 0;
	queue->rear = queue->capacity - 1;
}

int queue_init(belavin_queue_t *queue, int type) {
	queue_reset(queue);
	g_mutex_init(&queue->mutex);
	g_cond_init(&queue->data_arrived);
	queue->type = type;
	int i;
	for (i = 0; i < MAX_QUEUE_DEPTH; ++i) {
		queue->items[i] = g_malloc(1024);
		queue->item_lens[i] = 0;
	}
	return 0;
}

int queue_is_full(belavin_queue_t *queue) {
	return (queue->size == queue->capacity);
}

int queue_is_empty(belavin_queue_t *queue) {
	return (queue->size == 0);
}

gint queue_clear(belavin_queue_t *queue) {
	g_mutex_clear(&queue->mutex);
	g_cond_clear(&queue->data_arrived);
	return 0;
}

int queue_enqueue(belavin_queue_t *queue, gpointer in, int inlen, int signal) {
	g_mutex_lock(&queue->mutex);

	if (queue_is_full(queue)) {
		g_mutex_unlock(&queue->mutex);
		return -1;
	}

	queue->rear = (queue->rear + 1) % queue->capacity;

	//queue->items[queue->rear] = item;
	queue->item_lens[queue->rear] = inlen;
	guint8 *dest = (guint8*) queue->items[queue->rear];
	guint8 *src = (guint8*) in;
	for (int i = 0; i < inlen; ++i) {
		dest[i] = src[i];
	}

	queue->size = queue->size + 1;

	if (signal == QUEUE_ENQUE_SIGNAL)
		g_cond_signal(&queue->data_arrived);

	g_mutex_unlock(&queue->mutex);
	g_print("enque finish\n");
	return 0;
}

int queue_dequeue(belavin_queue_t *queue, gpointer out, int *outlen) {
	g_mutex_lock(&queue->mutex);

	if (queue_is_empty(queue)) {
		g_mutex_unlock(&queue->mutex);
		return -1;
	}

	*outlen = queue->item_lens[queue->front];
	guint8 *src = (guint8*) queue->items[queue->front];
	guint8 *dest = (guint8*) out;
	for (int i = 0; i < queue->item_lens[queue->front]; ++i) {
		dest[i] = src[i];
	}

	queue->front = (queue->front + 1) % queue->capacity;

	queue->size = queue->size - 1;
	g_mutex_unlock(&queue->mutex);

	return 0;
}

void queue_dump(belavin_queue_t *queue) {
	g_mutex_lock(&queue->mutex);
	g_print("****** QUEUE front=%d, rear=%d, size=%d, capacity=%d type=%d\n",
			queue->front, queue->rear, queue->size, queue->capacity,
			queue->type);
	g_mutex_unlock(&queue->mutex);
}

int queue_wait(belavin_queue_t *queue, int secs) {
	int r;
	gint64 end_time;
	end_time = g_get_monotonic_time() + secs * G_TIME_SPAN_SECOND;

	g_mutex_lock(&queue->mutex);

	if (!g_cond_wait_until(&queue->data_arrived, &queue->mutex, end_time)) {
		g_print("output queue wait pdu timed out\n");
		r = -1;
		g_mutex_unlock(&queue->mutex);
	} else {
		r = queue->size;
		g_mutex_unlock(&queue->mutex);
	}
	return r;
}

int queue_get_size(belavin_queue_t *queue) {
	int r;
	g_mutex_lock(&queue->mutex);
	r = queue->size;
	g_mutex_unlock(&queue->mutex);
	return r;
}
