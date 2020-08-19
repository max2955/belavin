#ifndef BELAVIN_NETPDU
#define  BELAVIN_NETPDU 1

#include <glib.h>

#define NETPDUCMD_GETSIZE 0x101
#define NETPDUCMD_ERROR 0x100
#define NETPDUCMD_READ 0x202
#define NETPDUCMD_WRITE 0x303

#define NETPDUCMD_GETSIZE_RESP 0x102
#define NETPDUCMD_READ_RESP 0x203
#define NETPDUCMD_WRITE_RESP 0x304
#define NETPDUCMD_NACK_RESP 0x999

#define MAX_WRITE_DATA_LENGTH 512
#define NETPDUSTATUS_OK 0x101
#define NETPDUSTATUS_ERROR 0x202

typedef struct {
	guint32 length; //4
	guint32 cmd;    //4
	guint32 seq;    //4
	guint64 offset;  // 8
} __attribute__((packed)) netpdu_header_t;

typedef struct {
	netpdu_header_t header;
} __attribute__((packed)) netpdu_getsize_t;

typedef struct {
	netpdu_header_t header;

} __attribute__((packed)) netpdu_read_t;

typedef struct {
	netpdu_header_t header;
	guint8 block_data[MAX_WRITE_DATA_LENGTH];
} __attribute__((packed)) netpdu_write_t;

typedef struct {
	netpdu_header_t header;
	guint64 status;
} __attribute__((packed)) netpdu_getsize_resp_t;

typedef struct {
	netpdu_header_t header;
	guint64 status;
} __attribute__((packed)) netpdu_write_resp_t;

typedef struct {
	netpdu_header_t header;
	guint64 status;
} __attribute__((packed)) netpdu_nack_resp_t;

typedef struct {
	netpdu_header_t header;
	guint64 status;
	guint8 block_data[MAX_WRITE_DATA_LENGTH];
} __attribute__((packed)) netpdu_read_resp_t;

netpdu_getsize_t* new_netpdu_getsize(gpointer holder, guint32 seq);
netpdu_read_t* new_netpdu_read(gpointer holder, guint64 offset, guint32 seq);
netpdu_write_t* new_netpdu_write(gpointer holder, guint64 offset, guint8 *data,
		guint32 seq);

netpdu_getsize_resp_t* new_netpdu_getsize_resp(gpointer holder, guint64 offset,
		guint32 seq, guint64 status);
netpdu_nack_resp_t* new_netpdu_nack_resp(gpointer holder, guint64 offset,
		guint32 seq, guint64 status);

netpdu_read_resp_t* new_netpdu_read_resp(gpointer holder, guint64 offset,
		guint8 *data, guint32 seq, guint64 status);
netpdu_write_resp_t* new_netpdu_write_resp(gpointer holder, guint64 offset,
		guint32 seq, guint64 status);

int netpdu_to_buf(gpointer pdu, guint8 *buffer, guint32 *buffer_len);
gpointer buf_to_netpdu(gpointer buf, guint32 buflen);
gchar* dump_netpdu(gpointer buffer, guint32 buffer_length);

#endif

