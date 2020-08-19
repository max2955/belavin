/*
 * stream.c
 *
 *  Created on: Jul 22, 2020
 *      Author: max
 */

#include "stream.h"

void stream_inc_ref_count(belavin_stream_t *stream) {
	g_mutex_lock(&stream->mutex);
	++stream->ref_count;
	g_mutex_unlock(&stream->mutex);
}

void stream_dec_ref_count(belavin_stream_t *stream) {
	g_mutex_lock(&stream->mutex);
	--stream->ref_count;
	g_mutex_unlock(&stream->mutex);
}

int stream_get_ref_count(belavin_stream_t *stream) {
	int rc;
	g_mutex_lock(&stream->mutex);
	rc = stream->ref_count;
	g_mutex_unlock(&stream->mutex);
	return rc;
}
