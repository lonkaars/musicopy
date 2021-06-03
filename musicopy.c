#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ini.h>
#include <cwalk.h>
#include <ftw.h>
#include <fnmatch.h>
#include <wordexp.h>

char* music_dir;
char* playlist_dir;
char* target_music_dir;
char* target_playlist_dir;
char** exclude;
int exclude_length;
char** include;
int include_length;
char* existing;

char* config_section = "default";

char* join_path(char* a, char* b) {
	size_t size = (strlen(a) + strlen(b) + 4) * sizeof(char);
	char* fullpath = (char*) malloc(size);
	cwk_path_join(a, b, fullpath, size);
	return fullpath;
}

void append(char*** dest, int* length, char* add) {
	(*length)++;
	size_t new_size = *length * sizeof(char*);
	char** temp = malloc(new_size);
	for(int i = 0; i < *length -1; i++)
		temp[i] = *dest[i];
	temp[*length-1] = add;

	free(*dest);
	*dest = temp;
}

static int handler(void* user, const char* section, const char* name, const char* value) {
	if (strcmp(section, config_section) != 0) return 1;

	if      (0 == strcmp(name, "music_dir"))           music_dir           = strdup(value);
	else if (0 == strcmp(name, "playlist_dir"))        playlist_dir        = strdup(value);
	else if (0 == strcmp(name, "target_music_dir"))    target_music_dir    = strdup(value);
	else if (0 == strcmp(name, "target_playlist_dir")) target_playlist_dir = strdup(value);
	else if (0 == strcmp(name, "existing"))            existing            = strdup(value);
	else if (0 == strcmp(name, "include"))             append(&include, &include_length, strdup(value));
	else if (0 == strcmp(name, "exclude"))             append(&exclude, &exclude_length, strdup(value));

	return 1;
}

void exit_err(char* msg) {
	printf("%s exiting...\n", msg);
	exit(1);
}

void load_config() {
	char* home = getenv("HOME");
	if(!home) exit_err("$HOME could not be read!");

	char* config_file_loc = ".config/musicopy/rc.ini";
	size_t config_path_size = (strlen(home) + strlen(config_file_loc) + 4) * sizeof(char);
	char* config_path = malloc(config_path_size);
	cwk_path_join(home, config_file_loc, config_path, config_path_size);

	if( access(config_path, F_OK) != 0 ) exit_err("Config file could not be read!");

	exclude_length      = 0;
	include_length      = 0;
	existing            = strdup("");
	music_dir           = strdup("");
	playlist_dir        = strdup("");
	target_music_dir    = strdup("");
	target_playlist_dir = strdup("");

	if (ini_parse(config_path, handler, NULL) < 0) exit_err("Can't load configuration file!");
}

void expandpath(char** path) {
	wordexp_t run;
	wordexp(*path, &run, WRDE_NOCMD);
	free(*path);
	*path = strdup(run.we_wordv[0]);
}

void fix_include_exclude(char*** listv, int listc) {
	int music_dir_len = strlen(music_dir);
	for(int i = 0; i < listc; i++) {
		size_t new_path_size = (music_dir_len + strlen((*listv)[i]) + 4) * sizeof(char);
		char* new_path = (char*) malloc(new_path_size);
		cwk_path_join(music_dir, (*listv)[i], new_path, new_path_size);

		free((*listv)[i]);
		(*listv)[i] = new_path;
	}
}

void copy(const char* path) {
	printf("copying %s... to %s\n", path, target_music_dir);
}

int dir_callback(const char* path, const struct stat *sb, int tflag) {
	if (include_length > 0) {
		bool allowed = false;
		for(int i = 0; i < include_length; i++) {
			if(allowed) {
				copy(path);
				return 0;
			} else {
				allowed = fnmatch(exclude[i], path, 0) == 0;
			}
		}
		return 0;
	}

	copy(path);

	return 0;
}

void print_opts() {
	printf("{\n");
	printf("    \"music_dir\":           \"%s\",\n", music_dir);
	printf("    \"playlist_dir\":        \"%s\",\n", playlist_dir);
	printf("    \"target_music_dir\":    \"%s\",\n", target_music_dir);
	printf("    \"target_playlist_dir\": \"%s\",\n", target_playlist_dir);
	printf("    \"existing\":            \"%s\",\n", existing);
	printf("    \"include\": [\n");
	for(int i = 0; i < include_length; i++)
		printf("        \"%s\"%c\n", include[i], i+1 == include_length ? ' ' : ',');
	printf("    ],\n");
	printf("    \"exclude\": [\n");
	for(int i = 0; i < exclude_length; i++)
		printf("        \"%s\"%c\n", exclude[i], i+1 == exclude_length ? ' ' : ',');
	printf("    ]\n");
	printf("}\n");
}

int main() {
	load_config();

	expandpath(&music_dir);
	expandpath(&playlist_dir);

	expandpath(&target_music_dir);
	expandpath(&target_playlist_dir);

	fix_include_exclude(&include, include_length);

	fix_include_exclude(&exclude, exclude_length);

	print_opts();

	ftw(music_dir, dir_callback, 0);

	return 0;
}
