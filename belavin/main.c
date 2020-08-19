#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <sys/stat.h>
#include <glib/gstdio.h>
#include "configuration.h"
#include "data_interface.h"
#include "mgmt_interface.h"
#include "storage.h"
#include "storage_unit.h"
#include "stream_system.h"
#define G_LOG_DOMAIN "belavin-main"

int main() {

	belavin_stream_system_t *stream_system = get_stream_system();
	belavin_config_t *config = get_config();

	g_log_set_default_handler (g_log_default_handler,NULL) ;
	g_info("starting");

	if (-1 == prepare_config(config)) {
		g_print("Can not load config! Exit\n");
		return -1;
	}

	g_print("Config loaded\n");

	dump_config(config);

	stream_system_init(stream_system);
	stream_system_dump(stream_system);

	GThread *data_interface_thread_id;
	start_data_interface(&data_interface_thread_id);
	GThread *mgmt_interface_thread_id;
	start_mgmt_interface(&mgmt_interface_thread_id);
	GThread *storage_thread_id;
	start_storage(&storage_thread_id);

	g_thread_join(data_interface_thread_id);
	g_thread_join(mgmt_interface_thread_id);
	g_thread_join(storage_thread_id);


	stream_system_clear(stream_system);

	g_print("MAIN: stop\n");

	return 0;
}
