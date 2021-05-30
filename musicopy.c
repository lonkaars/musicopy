#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ini.h>
#include <cwalk.h>

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
	FILE* config_file_handle = fopen(config_path, "rw");
	long config_file_length;
	char* config_file_buffer = 0;

	fseek(config_file_handle, 0, SEEK_END);
	config_file_length = ftell(config_file_handle);
	fseek(config_file_handle, 0, SEEK_SET);
	config_file_buffer = malloc(config_file_length);
	if(!config_file_buffer) exit_err("An error occured allocating memory for the config file!");
	fread(config_file_buffer, 1, config_file_length, config_file_handle);

	printf("%s\n", config_path);
	printf("%s\n", config_file_buffer);
	return 0;
}
