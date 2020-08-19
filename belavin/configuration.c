#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include "configuration.h"
#include "connection.h"
#include "stream_system.h"

int data_port = 0;
gchar storage_unit[1024 + 1];

static belavin_config_t g_config;

belavin_config_t* get_config() {
	return &g_config;
}

gchar* get_config_line(GKeyFile *key_file, gchar *cat, gchar *key,
		GError *error) {

	gchar *val = g_key_file_get_string(key_file, cat, key, &error);

	if (val == NULL
			&& !g_error_matches(error, G_KEY_FILE_ERROR,
					G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
		g_print("Error finding key %s in config file: %s\n", key,
				error->message);
		return NULL;
	}
	return val;
}

int dump_config(belavin_config_t *cfg) {
	g_print("********************** CONFIG ******************\n");
	g_print("data_port=%d\n", cfg->data_port);
	g_print("storage_unit=%s\n", cfg->storage_unit);
	g_print(
			"storage_unit %s: type is %x, %ld 512B blocks sizeof(st_blocks)=%ld\n",
			cfg->storage_unit, cfg->storage_unit_stat.st_mode,
			cfg->storage_unit_stat.st_blocks,
			sizeof(cfg->storage_unit_stat.st_blocks));
	g_print("********************** CONFIG ******************\n");
	return 0;
}

int prepare_config(belavin_config_t *cfg) {

	cfg->stop_flag = 0;
	cfg->client_connect_num = 0;
	g_mutex_init(&cfg->mutex);


	gchar cfg_filepath[2048 + 1];
	/*g_snprintf(cfg_filepath, 1024, "/etc/%s", CFG_FILENAME);

	if (!g_file_test(cfg_filepath, G_FILE_TEST_EXISTS)) {
		g_snprintf(cfg_filepath, 1024, "./%s", CFG_FILENAME);
	}*/

	g_snprintf(cfg_filepath, 1024, "./%s", CFG_FILENAME);

	GError *error = NULL;
	GKeyFile *key_file = g_key_file_new();

	g_print("Load config from %s\n", cfg_filepath);

	if (!g_key_file_load_from_file(key_file, cfg_filepath, G_KEY_FILE_NONE,
			&error)) {
		if (!g_error_matches(error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
			g_print("Error loading key file: %s\n", error->message);
		return -1;
	}

	gchar *temp;

	temp = get_config_line(key_file, "main", "data_port", error);

	if (temp) {
		cfg->data_port = (int) g_ascii_strtoll(temp, NULL, 10);
	}

	g_free(temp);

	temp = get_config_line(key_file, "main", "mgmt_port", error);

	if (temp) {
		cfg->mgmt_port = (int) g_ascii_strtoll(temp, NULL, 10);
	}

	g_free(temp);

	temp = get_config_line(key_file, "main", "storage_unit", error);

	if (temp) {
		g_strlcpy(cfg->storage_unit, temp, 1024);
	}

	g_free(temp);

	g_stat(cfg->storage_unit, &cfg->storage_unit_stat);

	return 0;
}

