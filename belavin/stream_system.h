#ifndef BELAVIN_STREAM_SYSTEM
#define BELAVIN_STREAM_SYSTEM 1

#include <glib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "netpdu.h"

#include "configuration.h"
#include "queue.h"

#define MAX_STREAM_NUM 8
#define SLOT_ACTIVE 1
#define SLOT_FREE 2

#include "stream.h"

typedef struct {
	int slot_states[MAX_STREAM_NUM];
	belavin_stream_t *streams[MAX_STREAM_NUM];
	GMutex mutex;
	int active_slots;
} belavin_stream_system_t;

belavin_stream_system_t* get_stream_system();
gint stream_system_init(belavin_stream_system_t *stream_system);
gint stream_system_clear(belavin_stream_system_t *stream_system);
gint stream_system_get_stream(belavin_stream_system_t *stream_system);
void stream_system_dump(belavin_stream_system_t *stream_system);
void stream_system_release_stream(belavin_stream_system_t *stream_system,
		int idx);

#endif // BELAVIN_STREAM
