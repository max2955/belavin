#ifndef BELAVIN_NETWORK
#define BELAVIN_NETWORK
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "configuration.h"

int start_listen(int port, int *socket);
//int start_listen(int port);
int socket_non_block(int *socket);
//int socket_non_block(int socket);

int build_fd_set_all(int *socket, fd_set *read_fds, fd_set *write_fds,
		fd_set *except_fds);
int build_fd_set(int *socket, fd_set *fds);
int select_for_read(int *socket, int secs);
int select_for_write(int *socket, int secs);
int read_n(int *socket, guint8 *buf, int n, int secs);
int write_n(int *socket, guint8 *buf, int n, int secs);

#endif // BELAVIN_NETWORK
