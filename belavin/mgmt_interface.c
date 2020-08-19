/*
 * mgmt_interface.c
 *
 *  Created on: Jul 19, 2020
 *      Author: max
 */
#include <glib.h>
#include <glib/gstdio.h>
#include "configuration.h"
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

gpointer mgmt_interface_thread_func(gpointer data) {
	g_print("mgmt interface started\n");

	belavin_config_t *config = get_config();

	int listen_socket = 0, connfd = 0;
	char recvBuf[1025];

	memset(recvBuf, 0, sizeof(recvBuf));

	start_listen(config->mgmt_port, &listen_socket);

	g_print("start listen mgmt port\n");

	while (config->stop_flag != 1) {

		int rv;

		rv = select_for_read(&listen_socket, 1);

		if (rv == -1) {
			g_print("select error \n");
			continue;
		} else if (rv == 0) {
			//g_print("timeout occurred \n");
			continue;
		} else {
			struct sockaddr_in client_sockaddr;
			socklen_t client_addr_len;
			char str[INET_ADDRSTRLEN+1];
            memset(str,0,INET_ADDRSTRLEN) ;
			connfd = accept(listen_socket, (struct sockaddr*) &client_sockaddr,
					&client_addr_len);
			inet_ntop(AF_INET, &client_sockaddr.sin_addr.s_addr, str,
			INET_ADDRSTRLEN);

			g_print("mgmt: connected %s:%d\n", str, client_sockaddr.sin_port);

			int n = read(connfd, recvBuf, sizeof(recvBuf));

			if (n > 0) {
				recvBuf[n] = 0;
				if (g_ascii_strncasecmp(recvBuf, "STOP", 4) == 0)
					config->stop_flag = 1;
			}
			close(connfd);

		}
	}

	close(listen_socket);
	g_print("mgmt interface thread finished\n");
	return NULL;
}

int start_mgmt_interface(GThread **mgmt_interface_thread_id) {
	*mgmt_interface_thread_id = g_thread_new("mgmt", mgmt_interface_thread_func,
	NULL);
	return 0;
}
