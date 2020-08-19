#include "network.h"
#include "configuration.h"

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>

int start_listen(int port, int *listen_socket) {
	struct sockaddr_in serv_addr;

	*listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	socket_non_block(listen_socket);

	bind(*listen_socket, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	listen(*listen_socket, 10);

	return 0;
}

int socket_non_block(int *socket) {
	int flags = fcntl(*socket, F_GETFL);
	return fcntl(*socket, F_SETFL, flags | O_NONBLOCK);
}

int build_fd_set_all(int *socket, fd_set *read_fds, fd_set *write_fds,
		fd_set *except_fds) {

	if (socket == NULL)
		return -1;

	if (read_fds != NULL) {
		FD_ZERO(read_fds);
		FD_SET(*socket, read_fds);
	}

	if (write_fds != NULL) {
		FD_ZERO(write_fds);
		FD_SET(*socket, write_fds);
	}

	if (except_fds != NULL) {
		FD_ZERO(except_fds);
		FD_SET(*socket, except_fds);
	}

	return 0;
}

int build_fd_set(int *socket, fd_set *fds) {
	if (socket == NULL)
		return -1;
	if (fds != NULL) {
		FD_ZERO(fds);
		FD_SET(*socket, fds);
	}
	return 0;
}

int select_for_read(int *socket, int secs) {
	fd_set read_fds;
	build_fd_set(socket, &read_fds);

	struct timeval select_timeout;
	select_timeout.tv_sec = secs;
	select_timeout.tv_usec = 0;

	return select(*socket + 1, &read_fds, NULL, NULL, &select_timeout);
}

int select_for_write(int *socket, int secs) {
	fd_set write_fds;
	build_fd_set(socket, &write_fds);

	struct timeval select_timeout;
	select_timeout.tv_sec = secs;
	select_timeout.tv_usec = 0;

	return select(*socket + 1, NULL, &write_fds, NULL, &select_timeout);
}

int read_n(int *socket, guint8 *buf, int n, int secs) {
	gint64 start_time = g_get_monotonic_time();
	gint64 end_time = start_time + secs * G_TIME_SPAN_SECOND;
	gint64 current_time = start_time;

	guint8 *buf_start = buf;
	int howmany = n;
	int r = -1;

	while (1) {
		int select_timeout_sec = 1;
		r = select_for_read(socket, select_timeout_sec);

		if (r == -1) {
			perror("select error in read_n\n");
			break;
		} else if (r == 0) {
			//g_print("timeout occurred \n");
			current_time = current_time + G_TIME_SPAN_SECOND;
			if (current_time < end_time)
				continue;
			else {
				r = 0;
				break;
			}
		}

		int nread = read(*socket, buf_start, howmany);

		if (nread == howmany) {
			r = n;
			break;
		}

		if (0 == nread) {
			// socket connection is going down
			r = -1;
			break;
		}

		if (-1 == nread) {
			if (errno == EAGAIN)
				continue;
			else {
				// error
				r = -1;
				break;
			}
		}

		if (nread > 0 && nread < howmany) {
			r = nread;
			buf_start = buf_start + nread;
			howmany = howmany - nread;
			continue;
		}

	}

	return r;
}

int write_n(int *socket, guint8 *buf, int n, int secs) {
	gint64 start_time = g_get_monotonic_time();
	gint64 end_time = start_time + secs * G_TIME_SPAN_SECOND;
	gint64 current_time = start_time;

	guint8 *buf_start = buf;
	int howmany = n;
	int r = -1;

	while (1) {
		int select_timeout_sec = 1;
		r = select_for_write(socket, select_timeout_sec);

		if (r == -1) {
			perror("select error in write_n\n");
			break;
		} else if (r == 0) {
			//g_print("timeout occurred \n");
			current_time = current_time + G_TIME_SPAN_SECOND;
			if (current_time < end_time)
				continue;
			else {
				r = 0;
				break;
			}
		}

		int nwrite = write(*socket, buf_start, howmany);

		if (nwrite == howmany) {
			r = n;
			break;
		}

		if (0 == nwrite) {
			//socket connection is going down
			r = -1;
			break;
		}

		if (-1 == nwrite) {
			if (errno == EAGAIN)
				continue;
			else {
				r = -1;
				break;
			}
		}

		if (nwrite > 0 && nwrite < howmany) {
			buf_start = buf_start + nwrite;
			howmany = howmany - nwrite;
			continue;
		}

	}

	return r;

}

