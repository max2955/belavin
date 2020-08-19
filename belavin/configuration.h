#ifndef BELAVIN_CONFIGURATION
#define BELAVIN_CONFIGURATION 1

#include <glib.h>
#include <glib/gstdio.h>

#define CFG_FILENAME "belavin.cfg"

typedef struct {
	guint data_port;
	guint mgmt_port;
	gchar storage_unit[1024 + 1];
	GStatBuf storage_unit_stat;
	volatile int stop_flag;
	volatile int client_connect_num;
	GMutex mutex;
} belavin_config_t;

belavin_config_t* get_config();
int prepare_config();
int dump_config();

#endif
