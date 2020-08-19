#include <errno.h>

#include "storage.h"
#include "configuration.h"
#include "stream.h"
#include "stream_system.h"
#include "netpdu.h"
#include "storage_unit.h"

int process_pdu(gpointer pdu, int stream_idx) {
	netpdu_header_t *input_header = (netpdu_header_t*) pdu;
	belavin_config_t *config = get_config();
	belavin_stream_system_t *stream_system = get_stream_system();
	storage_unit_t *storage_unit = storage_unit_get();

	g_print("DEQUE length=%d cmd=%x seq=%d offset=%ld\n", input_header->length,
			input_header->cmd, input_header->seq, input_header->offset);

	guint8 new_pdu_buf[1024];
	guint8 new_send_buf[1024];

	netpdu_getsize_resp_t *gsr;

	netpdu_read_resp_t *rr;
	netpdu_write_resp_t *wr;

	netpdu_write_t *ww;

	guint8 tmp_buffer[512];
	guint8 *tmp_read_pointer = &tmp_buffer[0];

	switch (input_header->cmd) {
	case NETPDUCMD_GETSIZE:
		gsr = new_netpdu_getsize_resp(new_pdu_buf,
				(config->storage_unit_stat.st_blocks * 512), input_header->seq,
				NETPDUSTATUS_OK);
		netpdu_to_buf(gsr, new_send_buf, &gsr->header.length);
		queue_enqueue(&stream_system->streams[stream_idx]->output_queue,
				new_send_buf, gsr->header.length, QUEUE_ENQUE_SIGNAL);
		g_print("enqued response get size\n");
		break;
	case NETPDUCMD_READ:
		storage_unit_seek(storage_unit, input_header->offset);
		storage_unit_read(storage_unit, &tmp_read_pointer);
		rr = new_netpdu_read_resp(new_pdu_buf, input_header->offset, tmp_buffer,
				input_header->seq, NETPDUSTATUS_OK);
		netpdu_to_buf(rr, new_send_buf, &rr->header.length);
		queue_enqueue(&stream_system->streams[stream_idx]->output_queue,
				new_send_buf, rr->header.length, QUEUE_ENQUE_SIGNAL);
		g_print("enqued response read\n");
		break;
	case NETPDUCMD_WRITE:
		ww = (netpdu_write_t*) pdu;
		storage_unit_seek(storage_unit, input_header->offset);
		storage_unit_write(storage_unit, ww->block_data);
		wr = new_netpdu_write_resp(new_pdu_buf, input_header->offset,
				input_header->seq, NETPDUSTATUS_OK);
		netpdu_to_buf(wr, new_send_buf, &wr->header.length);
		queue_enqueue(&stream_system->streams[stream_idx]->output_queue,
				new_send_buf, wr->header.length, QUEUE_ENQUE_SIGNAL);
		g_print("enqued response write\n");
		break;
	}
	return 0;
}

gpointer storage_thread_func(gpointer data) {
	g_print("storage thread started\n");
	belavin_stream_system_t *stream_system = get_stream_system();
	belavin_config_t *config = get_config();
	storage_unit_t *storage_unit = storage_unit_get();

	storage_unit_init(storage_unit);

	storage_unit_prepare(storage_unit, config->storage_unit);

	while (config->stop_flag != 1) {
		while ((storage_unit_get_pending_count(storage_unit) < 1)
				&& (config->stop_flag != 1))
			storage_unit_wait_req(storage_unit, 5);

		if (config->stop_flag == 1) {
			break;
		}
		while (storage_unit_get_pending_count(storage_unit) > 0) {
			for (int i = 0; i < MAX_STREAM_NUM; ++i) {
				belavin_stream_t *stream = stream_system->streams[i];
				gint stream_ref_count;
				stream_ref_count = stream_get_ref_count(stream);
				if (stream_ref_count != ACTIVE_STREAM_REF_COUNT)
					continue;

				guint8 buf[1024];
				int len;
				int d = queue_dequeue(&stream->input_queue, buf, &len);
				if (d != -1) {
					process_pdu((gpointer) buf, i);
					storage_unit_dec_pending_count(storage_unit);
				}
			}
		}
	}

	storage_unit_flush(storage_unit);
	storage_unit_release(storage_unit);
	g_print("storage thread stopped\n");
	return NULL;
}

int start_storage(GThread **storage_thread_id) {

	*storage_thread_id = g_thread_new("storage", storage_thread_func, NULL);

	return 0;
}
