#include "netpdu.h"

#include <netinet/in.h>

guint64 ntoh64(const guint64 *input) {
	guint64 rval;
	guint8 *data = (guint8*) &rval;

	data[0] = *input >> 56;
	data[1] = *input >> 48;
	data[2] = *input >> 40;
	data[3] = *input >> 32;
	data[4] = *input >> 24;
	data[5] = *input >> 16;
	data[6] = *input >> 8;
	data[7] = *input >> 0;

	return rval;
}

guint64 hton64(const guint64 *input) {
	return (ntoh64(input));
}

netpdu_getsize_t* new_netpdu_getsize(gpointer holder, guint32 seq) {
	netpdu_getsize_t *pdu;
	if (holder == NULL)
		return NULL;
	pdu = (netpdu_getsize_t*) holder;
	pdu->header.length = 20;
	pdu->header.cmd = NETPDUCMD_GETSIZE;
	pdu->header.seq = seq;
	pdu->header.offset = 0;
	return pdu;
}

netpdu_read_t* new_netpdu_read(gpointer holder, guint64 offset, guint32 seq) {
	netpdu_read_t *pdu;
	if (holder == NULL)
		return NULL;
	pdu = (netpdu_read_t*) holder;
	pdu->header.length = 20;
	pdu->header.cmd = NETPDUCMD_READ;
	pdu->header.seq = seq;
	pdu->header.offset = offset;
	return pdu;
}

netpdu_write_t* new_netpdu_write(gpointer holder, guint64 offset,
		guint8 *block_data, guint32 seq) {
	netpdu_write_t *pdu;
	if (holder == NULL)
		return NULL;
	pdu = (netpdu_write_t*) holder;
	pdu->header.length = 20 + MAX_WRITE_DATA_LENGTH;
	pdu->header.cmd = NETPDUCMD_WRITE;
	pdu->header.seq = seq;
	pdu->header.offset = offset;
	int i;
	for (i = 0; i < MAX_WRITE_DATA_LENGTH; ++i)
		pdu->block_data[i] = block_data[i];
	return pdu;
}

netpdu_getsize_resp_t* new_netpdu_getsize_resp(gpointer holder, guint64 offset,
		guint32 seq, guint64 status) {

	//netpdu_header_t header ;
	//guint64 status ;
	netpdu_getsize_resp_t *pdu;
	if (holder == NULL)
		return NULL;
	pdu = (netpdu_getsize_resp_t*) holder;
	pdu->header.length = 28;
	pdu->header.cmd = NETPDUCMD_GETSIZE_RESP;
	pdu->header.seq = seq;
	pdu->header.offset = offset;
	pdu->status = NETPDUSTATUS_OK;
	return pdu;
}

netpdu_nack_resp_t* new_netpdu_nack_resp(gpointer holder, guint64 offset,
		guint32 seq, guint64 status) {

	netpdu_nack_resp_t *pdu;
	if (holder == NULL)
		return NULL;
	pdu = (netpdu_nack_resp_t*) holder;
	pdu->header.length = 28;
	pdu->header.cmd = NETPDUCMD_NACK_RESP;
	pdu->header.seq = seq;
	pdu->header.offset = offset;
	pdu->status = NETPDUSTATUS_ERROR;
	return pdu;
}

netpdu_read_resp_t* new_netpdu_read_resp(gpointer holder, guint64 offset,
		guint8 *block_data, guint32 seq, guint64 status) {
//    netpdu_header_t header ;
//    guint64 status ;
//    guint8 data[MAX_WRITE_DATA_LENGTH];

	netpdu_read_resp_t *pdu;
	if (holder == NULL)
		return NULL;
	pdu = (netpdu_read_resp_t*) holder;
	pdu->header.length = 28 + MAX_WRITE_DATA_LENGTH;
	pdu->header.cmd = NETPDUCMD_READ_RESP;
	pdu->header.seq = seq;
	pdu->header.offset = offset;
	pdu->status = NETPDUSTATUS_OK;

	for (int i = 0; i < MAX_WRITE_DATA_LENGTH; ++i) {
		pdu->block_data[i] = block_data[i];
	}

	return pdu;
}

netpdu_write_resp_t* new_netpdu_write_resp(gpointer holder, guint64 offset,
		guint32 seq, guint64 status) {
	//netpdu_header_t header ;
	//guint64 status ;
	netpdu_write_resp_t *pdu;
	if (holder == NULL)
		return NULL;
	pdu = (netpdu_write_resp_t*) holder;
	pdu->header.length = 28;
	pdu->header.cmd = NETPDUCMD_WRITE_RESP;
	pdu->header.seq = seq;
	pdu->header.offset = offset;
	pdu->status = NETPDUSTATUS_OK;
	return pdu;
}

int netpdu_to_buf(gpointer pdu, guint8 *buffer, guint32 *buffer_len) {

	netpdu_header_t *header = (netpdu_header_t*) pdu;
	netpdu_getsize_resp_t *getsize_resp_pdu;
	netpdu_nack_resp_t *nack_resp_pdu;
	netpdu_write_resp_t *write_resp_pdu;
	netpdu_write_t *write_pdu;
	netpdu_read_resp_t *read_resp_pdu;
	guint8 *block_data;

	guint32 *tmp = (guint32*) buffer;

	*tmp = htonl(header->length);
	++tmp;
	*tmp = htonl(header->cmd);
	++tmp;
	*tmp = htonl(header->seq);
	++tmp;
	*(guint64*) tmp = hton64(&(header->offset));
	++tmp;
	++tmp;

	switch (header->cmd) {

	case NETPDUCMD_GETSIZE:
		*buffer_len = 20;
		break;

	case NETPDUCMD_GETSIZE_RESP:
		getsize_resp_pdu = (netpdu_getsize_resp_t*) pdu;
		*buffer_len = 28;
		*(guint64*) tmp = hton64(&(getsize_resp_pdu->status));
		break;

	case NETPDUCMD_NACK_RESP:
		nack_resp_pdu = (netpdu_nack_resp_t*) pdu;
		*buffer_len = 28;
		*(guint64*) tmp = hton64(&(nack_resp_pdu->status));
		break;

	case NETPDUCMD_WRITE_RESP:
		write_resp_pdu = (netpdu_write_resp_t*) pdu;
		*buffer_len = 28;
		*(guint64*) tmp = hton64(&(write_resp_pdu->status));
		break;

	case NETPDUCMD_READ_RESP:
		read_resp_pdu = (netpdu_read_resp_t*) pdu;
		*buffer_len = 28 + MAX_WRITE_DATA_LENGTH;
		*(guint64*) tmp = hton64(&(read_resp_pdu->status));
		++tmp;
		++tmp;
		block_data = (guint8*) tmp;
		for (int i = 0; i < MAX_WRITE_DATA_LENGTH; ++i) {
			*block_data = read_resp_pdu->block_data[i];
			++block_data;
		}
		break;

	case NETPDUCMD_READ:
		*buffer_len = 20;
		break;

	case NETPDUCMD_WRITE:
		write_pdu = (netpdu_write_t*) pdu;
		*buffer_len = 20 + MAX_WRITE_DATA_LENGTH;
		block_data = (guint8*) tmp;
		for (int i = 0; i < MAX_WRITE_DATA_LENGTH; ++i) {
			*block_data = write_pdu->block_data[i];
			++block_data;
		}
		break;
	}

	return 0;
}

gpointer buf_to_netpdu(gpointer buf, guint32 buflen) {
	gpointer pdu;
	guint32 *tmp = (guint32*) buf;

	guint8 *block_data;

	guint32 length = ntohl(*tmp);
	++tmp;
	guint32 cmd = ntohl(*tmp);
	++tmp;
	guint32 seq = ntohl(*tmp);
	++tmp;
	guint64 offset = ntoh64(((guint64*) tmp));
	++tmp;
	++tmp;

	switch (cmd) {
	case NETPDUCMD_GETSIZE:
		pdu = (netpdu_getsize_t*) g_new(netpdu_getsize_t, 1);
		netpdu_getsize_t *getsize_pdu = (netpdu_getsize_t*) pdu;
		getsize_pdu->header.length = length;
		getsize_pdu->header.cmd = cmd;
		getsize_pdu->header.seq = seq;
		getsize_pdu->header.offset = offset;
		//*pdu_type = NETPDUCMD_GETSIZE;
		break;

	case NETPDUCMD_GETSIZE_RESP:
		pdu = (netpdu_getsize_resp_t*) g_new(netpdu_getsize_resp_t, 1);
		netpdu_getsize_resp_t *getsize_resp_pdu = (netpdu_getsize_resp_t*) pdu;
		getsize_resp_pdu->header.length = length;
		getsize_resp_pdu->header.cmd = cmd;
		getsize_resp_pdu->header.seq = seq;
		getsize_resp_pdu->header.offset = offset;
		getsize_resp_pdu->status = ntoh64(((guint64*) tmp));
		//*pdu_type = NETPDUCMD_GETSIZE_RESP;
		break;

	case NETPDUCMD_READ:
		pdu = (netpdu_read_t*) g_new(netpdu_read_t, 1);
		netpdu_read_t *read_pdu = (netpdu_read_t*) pdu;
		read_pdu->header.length = length;
		read_pdu->header.cmd = cmd;
		read_pdu->header.seq = seq;
		read_pdu->header.offset = offset;
		//*pdu_type = NETPDUCMD_READ;
		break;

	case NETPDUCMD_READ_RESP:
		pdu = (netpdu_read_resp_t*) g_new(netpdu_read_resp_t, 1);
		netpdu_read_resp_t *read_resp_pdu = (netpdu_read_resp_t*) pdu;
		read_resp_pdu->header.length = length;
		read_resp_pdu->header.cmd = cmd;
		read_resp_pdu->header.seq = seq;
		read_resp_pdu->header.offset = offset;
		read_resp_pdu->status = ntoh64(((guint64*) tmp));
		++tmp;
		++tmp;
		block_data = (guint8*) tmp;
		for (int i = 0; i < MAX_WRITE_DATA_LENGTH; ++i) {
			read_resp_pdu->block_data[i] = *block_data;
			++block_data;
		}
		//*pdu_type = NETPDUCMD_READ_RESP;
		break;

	case NETPDUCMD_WRITE:
		pdu = (netpdu_write_t*) g_new(netpdu_write_t, 1);
		netpdu_write_t *write_pdu = (netpdu_write_t*) pdu;
		write_pdu->header.length = length;
		write_pdu->header.cmd = cmd;
		write_pdu->header.seq = seq;
		write_pdu->header.offset = offset;
		//*pdu_type = NETPDUCMD_WRITE;
		block_data = (guint8*) tmp;
		for (int i = 0; i < MAX_WRITE_DATA_LENGTH; ++i) {
			write_pdu->block_data[i] = *block_data;
			++block_data;
		}
		break;

	case NETPDUCMD_WRITE_RESP:
		pdu = (netpdu_write_resp_t*) g_new(netpdu_write_resp_t, 1);
		netpdu_write_resp_t *write_resp_pdu = (netpdu_write_resp_t*) pdu;
		write_resp_pdu->header.length = length;
		write_resp_pdu->header.cmd = cmd;
		write_resp_pdu->header.seq = seq;
		write_resp_pdu->header.offset = offset;
		write_resp_pdu->status = ntoh64(((guint64*) tmp));
		//*pdu_type = NETPDUCMD_WRITE_RESP;
		break;
	}
	return pdu;
}

gchar* dump_netpdu(gpointer buffer, guint32 buffer_length) {
	gpointer ret = g_malloc(buffer_length * 2 + 1);
	gsize i;
	for (i = 0; i < buffer_length; i++) {

		g_snprintf((gchar*) (ret + i * 2), 3, "%02x",
				(guint) (*((guint8*) (buffer + i))));
	}
	return ret;
}
