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

void print_opts(config_file_opts options) {
	printf("{\n");
	printf("    \"music_dir\":    \"%s\",\n", options.music_dir);
	printf("    \"playlist_dir\": \"%s\",\n", options.playlist_dir);
	printf("    \"existing\":     \"%s\",\n", options.existing);
	printf("    \"include\":      \"%s\",\n", options.include);
	printf("    \"exclude\":      \"%s\"\n",  options.exclude);
	printf("}\n");
}

void append_with_nl(char** dest, char* add) {
	int len = strlen(*dest) + strlen(add) + 1;
	char* temp = (char*) malloc(len * sizeof(char));
	sprintf(temp, "%s%s\n", *dest, add);

	free(*dest);
	*dest = strdup(temp);
	free(temp);
}

static int handler(void* user, const char* section, const char* name, const char* value) {
	config_file_opts* pconfig = (config_file_opts*)user;
	if (strcmp(section, config_section) != 0) return 1;

	if      (0 == strcmp(name, "music_dir"))    pconfig->music_dir    = strdup(value);
	else if (0 == strcmp(name, "playlist_dir")) pconfig->playlist_dir = strdup(value);
	else if (0 == strcmp(name, "existing"))     pconfig->existing     = strdup(value);
	else if (0 == strcmp(name, "include"))      append_with_nl(&pconfig->include, strdup(value));
	else if (0 == strcmp(name, "exclude"))      append_with_nl(&pconfig->exclude, strdup(value));

	return 1;
}

void exit_err(char* msg) {
	printf("%s exiting...\n", msg);
	exit(1);
}

config_file_opts load_config() {
	char* home = getenv("HOME");
	if(!home) exit_err("$HOME could not be read!");

	char* config_file_loc = ".config/musicopy/rc.ini";
	size_t config_path_size = (strlen(home) + strlen(config_file_loc) + 4) * sizeof(char);
	char* config_path = malloc(config_path_size);
	cwk_path_join(home, config_file_loc, config_path, config_path_size);

	if( access(config_path, F_OK) != 0 ) exit_err("Config file could not be read!");

	config_file_opts config = {
		.exclude      = strdup(""),
		.existing     = strdup(""),
		.include      = strdup(""),
		.music_dir    = strdup(""),
		.playlist_dir = strdup("")
	};
	if (ini_parse(config_path, handler, &config) < 0) exit_err("Can't load configuration file!");

	return config;
}

void loop_over_str_nl(char* str, void fn(char*)) {
	int len = strlen(str);
	char temp_str[len];
	int temp_len = 0;

	for(int i = 0; i < len; i++) {
		if (str[i] != '\n') {
			temp_len++;
			strncat(temp_str, &str[i], 1);
		} else {
			fn(temp_str);

			temp_len = 0;
			memcpy(temp_str, "", sizeof(char));
		}
	}
}

void callback(char* line) {
	printf("line: %s\n", line);
}

int main() {
	config_file_opts config = load_config();
	
	loop_over_str_nl(config.exclude, callback);

	print_opts(config);

	return 0;
}
