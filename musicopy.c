#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ini.h>
#include <cwalk.h>

typedef struct {
	const char* music_dir;
	const char* playlist_dir;
	const char* exclude;
	const char* include;
	const char* existing;
} config_file_opts;

char* config_section = "default";

static int handler(void* user, const char* section, const char* name, const char* value) {
	config_file_opts* pconfig = (config_file_opts*)user;
	printf("called :tada:\n");
	return 1;
}

void exit_err(char* msg) {
	printf("%s exiting...\n", msg);
	exit(1);
}

int main() {
	char* home = getenv("HOME");
	if(!home) exit_err("$HOME could not be read!");

	char* config_file_loc = ".config/musicopy/rc.ini";
	size_t config_path_size = (strlen(home) + strlen(config_file_loc) + 4) * sizeof(char);
	char* config_path = malloc(config_path_size);
	cwk_path_join(home, config_file_loc, config_path, config_path_size);

	if( access(config_path, F_OK) != 0 ) exit_err("Config file could not be read!");

	config_file_opts* config;
	if (ini_parse(config_path, handler, &config) < 0) exit_err("Can't load configuration file!");

	printf("%s\n", config_path);
	return 0;
}
