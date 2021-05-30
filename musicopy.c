#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ini.h>
#include <cwalk.h>

int main() {
	char* home = getenv("HOME");
	if(!home) {
		printf("$HOME could not be read! exiting...\n");
		exit(1);
	}

	char* config_file_loc = ".config/musicopy/rc.ini";
	size_t config_path_size = (strlen(home) + strlen(config_file_loc) + 4) * sizeof(char);
	char* config_path = malloc(config_path_size);
	cwk_path_join(home, config_file_loc, config_path, config_path_size);

	if( access(config_path, F_OK) != 0 ) {
		printf("Config file can't be read! exiting...\n");
		exit(1);
	}
	FILE* config_file_handle = fopen(config_path, "rw");
	long config_file_length;
	char* config_file_buffer = 0;

	fseek(config_file_handle, 0, SEEK_END);
	config_file_length = ftell(config_file_handle);
	fseek(config_file_handle, 0, SEEK_SET);
	config_file_buffer = malloc(config_file_length);
	if(!config_file_buffer) {
		printf("An error occured allocating memory for the config file! exiting...\n");
		exit(1);
	}
	fread(config_file_buffer, 1, config_file_length, config_file_handle);

	printf("%s\n", config_path);
	printf("%s\n", config_file_buffer);
	return 0;
}
