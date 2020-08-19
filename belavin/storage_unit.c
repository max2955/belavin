#include "storage_unit.h"


storage_unit_t g_storage_unit;

storage_unit_t* storage_unit_get() {
	return &g_storage_unit;
}

int storage_unit_init(storage_unit_t *storage_unit) {
	g_mutex_init(&storage_unit->mutex);
	storage_unit->pending_count = 0;
	g_cond_init(&storage_unit->req_arrived);
	return 0;
}

int storage_unit_inc_pending_count(storage_unit_t *storage_unit) {
	g_mutex_lock(&storage_unit->mutex);
	++storage_unit->pending_count;
	g_mutex_unlock(&storage_unit->mutex);

	return 0;
}

int storage_unit_signal_new_req(storage_unit_t *storage_unit) {
	g_mutex_lock(&storage_unit->mutex);
	++storage_unit->pending_count;
	g_cond_signal(&storage_unit->req_arrived);
	g_mutex_unlock(&storage_unit->mutex);
	return 0;
}

int storage_unit_get_pending_count(storage_unit_t *storage_unit) {
	int r;
	g_mutex_lock(&storage_unit->mutex);
	r = ++storage_unit->pending_count;
	g_mutex_unlock(&storage_unit->mutex);
	return r;
}

int storage_unit_dec_pending_count(storage_unit_t *storage_unit) {
	g_mutex_lock(&storage_unit->mutex);
	--storage_unit->pending_count;
	g_mutex_unlock(&storage_unit->mutex);
	return 0;
}

int storage_unit_wait_req(storage_unit_t *storage_unit, int secs) {
	int r;
	g_mutex_lock(&storage_unit->mutex);

	gint64 end_time;
	end_time = g_get_monotonic_time() + secs * G_TIME_SPAN_SECOND;

	if (!g_cond_wait_until(&storage_unit->req_arrived, &storage_unit->mutex,
			end_time)) {
		r = -1;
		g_mutex_unlock(&storage_unit->mutex);
	} else {
		r = storage_unit->pending_count;
		g_mutex_unlock(&storage_unit->mutex);
	}
	return r;
}


int storage_unit_prepare(storage_unit_t *storage_unit,gchar * unit_name)
{
	storage_unit->unit_file = fopen(unit_name, "r+");
	return 0;
}

int storage_unit_release(storage_unit_t *storage_unit)
{
	return fclose(storage_unit->unit_file);
}

int storage_unit_seek(storage_unit_t *storage_unit,guint64 offset)
{
	return fseek(storage_unit->unit_file, offset, SEEK_SET);
}

int storage_unit_read(storage_unit_t *storage_unit,guint8 ** data)
{
	return fread(*data, 512, 1, storage_unit->unit_file);
}

int storage_unit_write(storage_unit_t *storage_unit,guint8 * data)
{
	return fwrite(data, 512, 1, storage_unit->unit_file);
}

int storage_unit_flush(storage_unit_t *storage_unit)
{
	return fflush(storage_unit->unit_file);
}

