#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ini.h>
#include <cwalk.h>

typedef struct {
	char* music_dir;
	char* playlist_dir;
	char* exclude;
	char* include;
	char* existing;
} config_file_opts;

char* config_section = "default";

static int handler(void* user, const char* section, const char* name, const char* value) {
	config_file_opts* pconfig = (config_file_opts*)user;
	if (strcmp(section, config_section) != 0) return 1;

	if      (strcmp(name, "music_dir") == 0)    pconfig->music_dir    = strdup(value);
	else if (strcmp(name, "playlist_dir") == 0) pconfig->playlist_dir = strdup(value);
	else if (strcmp(name, "existing") == 0)     pconfig->existing     = strdup(value);
	else if (strcmp(name, "include") == 0) {
		int len = strlen(pconfig->include) + strlen(value) + 1;
		char* include = (char*) malloc(len * sizeof(char));
		sprintf(include, "%s%s\n", pconfig->include, value);

		free(pconfig->include);
		pconfig->include = strdup(include);
		free(include);
	}
	else if (strcmp(name, "exclude") == 0) {
		int len = strlen(pconfig->exclude) + strlen(value) + 1;
		char* exclude = (char*) malloc(len * sizeof(char));
		sprintf(exclude, "%s%s\n", pconfig->exclude, value);

		free(pconfig->exclude);
		pconfig->exclude = strdup(exclude);
		free(exclude);
	}

	return 1;
}

void exit_err(char* msg) {
	printf("%s exiting...\n", msg);
	exit(1);
}

void print_opts(config_file_opts options) {
	printf("{\n");
	printf("    \"music_dir\":    \"%s\",\n", options.music_dir);
	printf("    \"playlist_dir\": \"%s\",\n", options.playlist_dir);
	printf("    \"existing\":     \"%s\",\n", options.existing);
	printf("    \"include\":      \"%s\",\n", options.include);
	printf("    \"exclude\":      \"%s\"\n",  options.exclude);
	printf("}\n");
}

int main() {
	char* home = getenv("HOME");
	if(!home) exit_err("$HOME could not be read!");

	char* config_file_loc = ".config/musicopy/rc.ini";
	size_t config_path_size = (strlen(home) + strlen(config_file_loc) + 4) * sizeof(char);
	char* config_path = malloc(config_path_size);
	cwk_path_join(home, config_file_loc, config_path, config_path_size);

	if( access(config_path, F_OK) != 0 ) exit_err("Config file could not be read!");

	config_file_opts config;
	config.exclude = strdup("");
	config.include = strdup("");
	if (ini_parse(config_path, handler, &config) < 0) exit_err("Can't load configuration file!");

	print_opts(config);

	return 0;
}
