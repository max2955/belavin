#include <glib.h>
#include <glib/gstdio.h>

#include "netpdu.h"
#include "queue.h"
#include "stream.h"
#include "stream_system.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static belavin_stream_system_t g_stream_system;

belavin_stream_system_t* get_stream_system() {
	return &g_stream_system;
}

gint stream_system_init(belavin_stream_system_t *stream_system) {
	g_mutex_init(&stream_system->mutex);

	for (int i = 0; i < MAX_STREAM_NUM; ++i) {
		g_print("init stream slot %d\n", i);
		stream_system->slot_states[i] = SLOT_FREE;
		stream_system->streams[i] = (belavin_stream_t*) g_new(belavin_stream_t,
				1);
		queue_init(&stream_system->streams[i]->input_queue, QUEUE_TYPE_INPUT);
		queue_init(&stream_system->streams[i]->output_queue, QUEUE_TYPE_OUTPUT);
		g_mutex_init(&stream_system->streams[i]->mutex);
		stream_system->streams[i]->ref_count = 0;
	}
	return 0;
}

gint stream_system_clear(belavin_stream_system_t *stream_system) {
	int i;
	for (i = 0; i < MAX_STREAM_NUM; ++i) {
		queue_clear(&stream_system->streams[i]->input_queue);
		queue_clear(&stream_system->streams[i]->output_queue);
		g_free(stream_system->streams[i]);
	}
	g_mutex_clear(&stream_system->mutex);
	return 0;
}

gint stream_system_get_stream(belavin_stream_system_t *stream_system) {
	gint ret = -1;

	g_mutex_lock(&stream_system->mutex);

	for (int i = 0; i < MAX_STREAM_NUM; ++i) {
		if (stream_system->slot_states[i] == SLOT_FREE) {
			g_print(">>>locked slot %d\n", i);
			stream_system->slot_states[i] = SLOT_ACTIVE;
			++stream_system->active_slots;
			ret = i;
			break;
		}
	}
	g_mutex_unlock(&stream_system->mutex);

	return ret;
}

void stream_system_dump(belavin_stream_system_t *stream_system) {
	g_mutex_lock(&stream_system->mutex);
	int i;
	g_print("***********************************\n");
	for (i = 0; i < MAX_STREAM_NUM; ++i) {
		g_print("STREAM <index %d>  ref_count=%d\n", i,
				stream_system->streams[i]->ref_count);
		g_print("input queue\n");
		queue_dump(&stream_system->streams[i]->input_queue);
		g_print("output queue\n");
		queue_dump(&stream_system->streams[i]->output_queue);
	}
	g_print("***********************************\n");
	g_mutex_unlock(&stream_system->mutex);
}

void stream_system_release_stream(belavin_stream_system_t *stream_system,
		int idx) {
	g_mutex_lock(&stream_system->mutex);
	stream_system->slot_states[idx] = SLOT_FREE;
	--stream_system->active_slots;
	g_mutex_unlock(&stream_system->mutex);
}

