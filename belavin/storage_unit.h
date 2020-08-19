#ifndef STORAGE_UNIT_H_
#define STORAGE_UNIT_H_
#include <glib.h>
#include <stdio.h>

typedef struct {
	volatile int pending_count;
	GMutex mutex;
	GCond req_arrived;
	FILE *unit_file;

} storage_unit_t;

storage_unit_t* storage_unit_get();
int storage_unit_init(storage_unit_t *storage_unit);
int storage_unit_inc_pending_count(storage_unit_t *storage_unit);
int storage_unit_get_pending_count(storage_unit_t *storage_unit);
int storage_unit_dec_pending_count(storage_unit_t *storage_unit);
int storage_unit_signal_new_req(storage_unit_t *storage_unit);
int storage_unit_wait_req(storage_unit_t *storage_unit, int secs);
int storage_unit_prepare(storage_unit_t *storage_unit,gchar * unit_name);
int storage_unit_release(storage_unit_t *storage_unit);
int storage_unit_seek(storage_unit_t *storage_unit,guint64 offset);
int storage_unit_read(storage_unit_t *storage_unit,guint8 ** data);
int storage_unit_write(storage_unit_t *storage_unit,guint8 * data);
int storage_unit_flush(storage_unit_t *storage_unit);

#endif /* STORAGE_UNIT_H_ */
