#include <glib.h>
#include <glib/gstdio.h>
#include "configuration.h"
#include "queue.h"
#include "stream_system.h"
#include "connection.h"
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

gpointer data_interface_thread_func(gpointer data) {
	g_print("data interface started\n");
	belavin_stream_system_t *stream_system = get_stream_system();
	belavin_config_t *config = get_config();

	//volatile netpdu_nack_resp_t * nack_resp ;
	guint8 nack_resp_pdu_buf[32];
	static guint8 nack_resp_send_buf[32];
	guint32 nack_resp_buf_len;
	new_netpdu_nack_resp(nack_resp_pdu_buf, 0, 0, NETPDUSTATUS_ERROR);
	netpdu_to_buf(nack_resp_pdu_buf, nack_resp_send_buf, &nack_resp_buf_len);

	int listen_socket = 0, socket = 0;

	start_listen(config->data_port, &listen_socket);

	g_print("start listen data port\n");

	while (config->stop_flag != 1) {
		int rv = select_for_read(&listen_socket, 1);

		if (rv == -1) {
			perror("select error");
			continue;
		} else if (rv == 0) {
			//g_print("timeout occurred \n");
			continue;
		} else {
			struct sockaddr_in sockaddr;
			socklen_t addr_len;
			char str[INET_ADDRSTRLEN + 1];
			memset(str, 0, INET_ADDRSTRLEN);
			socket = accept(listen_socket, (struct sockaddr*) &sockaddr,
					&addr_len);
			inet_ntop(AF_INET, &sockaddr.sin_addr.s_addr, str,
			INET_ADDRSTRLEN);

			g_print("data connected %s:%d\n", str, sockaddr.sin_port);

			g_print("search available slot \n");

			gint slot = 0;

			slot = stream_system_get_stream(stream_system);

			if (slot < 0) {
				g_print("no available stream slot\n");
				//write(socket, "too many\n\0", 10);
				write_n(&socket, nack_resp_send_buf, nack_resp_buf_len, 5);
				close(socket);
			} else {
				g_print("available stream slot %d\n", slot);
				belavin_connection_t *connection = g_new(belavin_connection_t,
						1);
				connection_init(connection);
				connection->socket = socket;
				socket_non_block(&connection->socket);
				connection->addr.sin_family = AF_INET;
				connection->addr.sin_addr.s_addr = sockaddr.sin_addr.s_addr;
				connection->addr.sin_port = sockaddr.sin_port;
				connection->slot = slot;
				g_print("START CLIENT");
				GThread *connection_output_thread_id = g_thread_new(
						"client output", connection_output_thread_func,
						connection);
				GThread *connection_input_thread_id = g_thread_new(
						"client input", connection_input_thread_func,
						connection);
			}
		}
	}
	close(listen_socket);
	g_print("data interface thread finished\n");
	return NULL;
}

int start_data_interface(GThread **data_interface_thread_id) {
	//*interface_thread_id = g_thread_create(data_interface_thread_func,NULL,TRUE,NULL);
	*data_interface_thread_id = g_thread_new("data acceptor",
			data_interface_thread_func, NULL);
	return 0;
}

