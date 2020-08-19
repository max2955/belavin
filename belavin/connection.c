#include <glib.h>
#include <glib/gstdio.h>
#include "configuration.h"
#include "connection.h"
#include "stream_system.h"
#include "netpdu.h"
#include "network.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>

#include "storage_unit.h"

int connection_dec_ref_count(belavin_connection_t *connection) {
	int connection_ref_count;
	g_mutex_lock(&connection->mutex);
	--connection->ref_count;
	connection_ref_count = connection->ref_count;
	g_mutex_unlock(&connection->mutex);
	return connection_ref_count;
}

int connection_get_ref_count(belavin_connection_t *connection) {
	int connection_ref_count;
	g_mutex_lock(&connection->mutex);
	connection_ref_count = connection->ref_count;
	g_mutex_unlock(&connection->mutex);
	return connection_ref_count;
}

void connection_init(belavin_connection_t *connection) {
	g_mutex_init(&connection->mutex);
	connection->ref_count = ACTIVE_STREAM_REF_COUNT;
}

int check_pdu_len(guint32 pdu_len) {
	int r = 0;
	if ((pdu_len == 0) && (pdu_len > 600))
		r = -1;
	return r;
}

void release_connection(belavin_connection_t *connection) {
	belavin_stream_system_t *stream_system = get_stream_system();
	belavin_stream_t *stream = stream_system->streams[connection->slot];

	stream_dec_ref_count(stream);

	if (0 == connection_dec_ref_count(connection)) {
		g_print("close connection\n");
		queue_reset(&stream->input_queue);
		queue_reset(&stream->output_queue);
		close(connection->socket);
		stream_system_release_stream(stream_system, connection->slot);
		g_free(connection);
	}
}

gpointer connection_input_thread_func(gpointer data) {

	belavin_connection_t *connection = (belavin_connection_t*) data;
	g_print("client input thread started\n");
	belavin_config_t *config = get_config();
	belavin_stream_system_t *stream_system = get_stream_system();
	storage_unit_t *storage_unit = storage_unit_get();
	stream_inc_ref_count(stream_system->streams[connection->slot]);

	while (config->stop_flag != 1 && connection_get_ref_count(connection) == 2) {

		guint8 sendBuff[1024];

		//g_print("try read\n");
		int n = read_n(&connection->socket, sendBuff, 4, 5);

		//g_print("read n=%d\n", n);

		if (0 == n) {
			//g_print("nothing were read\n");
			continue;
		}

		if (-1 == n) {
			g_print("read_n error\n");
			break;
		}

		gint32 pdu_len = ntohl(*(gint32*) sendBuff);

		if (check_pdu_len(pdu_len) == -1) {
			//g_print("PDU len= %d failed\n", pdu_len);
			break;
		}

		//g_print("READ pdu_len = %d\n", pdu_len);

		n = read_n(&(connection->socket), sendBuff + 4, pdu_len - 4, 5);

		if (0 == n) {
			//g_print("nothing were read\n");
			continue;
		}

		if (-1 == n) {
			g_print("read_n error\n");
			break;
		}

		//g_print("read pdu body %d bytes\n", n);

		gpointer pdu = buf_to_netpdu((gpointer) sendBuff, 1024);
		netpdu_header_t *header = (netpdu_header_t*) pdu;

		//g_print("ENQUEUE length=%d cmd=0x%x seq=%d\n", header->length,
		//header->cmd, header->seq);

		queue_enqueue(&stream_system->streams[connection->slot]->input_queue,
				pdu, header->length, QUEUE_ENQUE_NOSIGNAL);
		storage_unit_signal_new_req(storage_unit);
	}

	g_print("STOP CLIENT Input\n");
	release_connection(connection);

	return NULL;
}

gpointer connection_output_thread_func(gpointer data) {

	belavin_connection_t *connection = (belavin_connection_t*) data;
	g_print("client ouput thread started\n");
	belavin_config_t *config = get_config();
	belavin_stream_system_t *stream_system = get_stream_system();
	//belavin_statistics_t * statistics = get_statistics();
	stream_inc_ref_count(stream_system->streams[connection->slot]);

	belavin_queue_t *output_queue =
			&stream_system->streams[connection->slot]->output_queue;

	int output_stop = 0;

	while ((config->stop_flag != 1) && connection_get_ref_count(connection) == 2) {
		g_print("output queue wait start\n");

		int output_count = 0;

		output_count = queue_wait(output_queue, 5);

		g_print("output count=%d\n", output_count);

		//break main cycle if we need to stop due to stop command after we begin to wait output
		if (config->stop_flag == 1)
			break;

		while (queue_get_size(output_queue) > 0) {
			//gpointer queue_dequeue(belavin_queue_t * queue,gpointer out, int * outlen)
			g_print("output queue before deque\n");

			int len;
			guint8 send_buf[1024];

			volatile int d = queue_dequeue(output_queue, send_buf, &len);
			g_print("output queue after deque d=%d\n", d);

			if (d != -1) {
				//int write_n(int * socket, guint8 * buf, int n,int secs)
				int n = write_n(&connection->socket, send_buf, len, 5);
				if (n < 1) {
					g_print("write_n failed\n");
					output_stop = 1;
					break;
				}
				g_print("write_n n=%d\n", n);
			}
		}

		//break main cycle if we failed to send data
		if (1 == output_stop)
			break;
	}
	release_connection(connection);
	g_print("STOP CLIENT Output\n");
	return NULL;
}
