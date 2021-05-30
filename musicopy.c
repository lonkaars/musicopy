#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

	printf("%s\n", config_path);
	return 0;
}
