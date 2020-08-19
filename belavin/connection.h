#ifndef BELAVIN_CONNECTION
#define BELAVIN_CONNECTION 1

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "queue.h"
#include "configuration.h"
#include "stream_system.h"

#define MAX_CONNECT_NUM 8

typedef struct {
	int socket;
	struct sockaddr_in addr;
	GMutex mutex;
	int slot;
	int ref_count;
} belavin_connection_t;

void connection_init(belavin_connection_t*);
gpointer connection_input_thread_func(gpointer data);
gpointer connection_output_thread_func(gpointer data);
void close_connection(belavin_connection_t*);

#endif // BELAVIN_CONNECTION
