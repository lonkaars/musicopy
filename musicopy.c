#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ini.h>
#include <cwalk.h>
#include <ftw.h>
#include <fnmatch.h>
#include <wordexp.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <argp.h>

char* music_dir;
char* playlist_dir;
char* target_music_dir;
char* target_playlist_dir;
char** exclude;
int exclude_length;
char** include;
int include_length;
char* existing;

int buffer_size = 4096;
char* config_section = "default";
bool dry_run = false;

char* playlist_prefix;
char* target_playlist_prefix;
char* change_playlist_extension;

const char *argp_program_version = "0.1.0";
const char *argp_program_bug_address = "https://github.com/lonkaars/musicopy/";
static char doc[] = "a simple utility that copies music and playlists";
static char args_doc[] = "arguments";
static struct argp_option options[] = { 
	{ "buffer_size",	'b',	"SIZE",		0,	"Buffer size (in bytes) used for copying, higher means less syscalls (faster) but more memory usage, lower is more syscalls (slower) but less memory usage"},
	{ "player",			'p',	"PLAYER",	0,	"Config section to use"},
	{ "dry_run",		'd',	0,			0,	"Don't copy files, just print what would be copied"},
	{ 0 } 
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
	switch (key) {
		case 'b': {
			buffer_size = atoi(arg);
			break;
		}
		case 'p': {
			config_section = strdup(arg);
			break;
		}
		case 'd': {
			dry_run = true;
			break;
		}
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

void mkpath(char* file_path, mode_t mode) {
    for (char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = '\0';
		//TODO: error handling
        mkdir(file_path, mode);
        *p = '/';
    }
}

void create_folder_for_file(char* file_path) {
	if(dry_run) return;

	size_t destfolder_size;
	cwk_path_get_dirname(file_path, &destfolder_size);
	char* destfolder = (char*) malloc(destfolder_size);
	destfolder = strncpy(destfolder, file_path, destfolder_size);
	mkpath(destfolder, 0775);
}

int cp(char* source_path, char* dest_path) {
	if(dry_run) return 0;

	FILE *source, *dest;
	source = fopen(source_path, "rb");
	dest   = fopen(dest_path,   "wb");

	if(source == NULL) {
		fprintf(stderr, "can't read source: \"%s\" skipping...\n", source_path);
		if(dest != NULL) fclose(dest);
		return 1;
	}
	if(dest == NULL) {
		fprintf(stderr, "can't write to destination: \"%s\" skipping...\n", dest_path);
		if(source != NULL) fclose(source);
		return 1;
	}

	char buffer[buffer_size];
	size_t size;

	while ((size = fread(buffer, 1, buffer_size, source)))
        fwrite(buffer, 1, size, dest);

	fclose(source);
	fclose(dest);

	return 0;
}

char* join_path(char* a, char* b) {
	size_t size = (strlen(a) + strlen(b) + 4) * sizeof(char);
	char* fullpath = (char*) malloc(size);
	cwk_path_join(a, b, fullpath, size);
	return fullpath;
}

char* substr(const char* src, int from) {
	int fulllen = strlen(src);
	char* substr = malloc((fulllen - from + 2) * sizeof(char));
	strcpy(substr, &src[from]);
	return substr;
}

// https://stackoverflow.com/questions/4770985/how-to-check-if-a-string-starts-with-another-string-in-c
bool starts_with(const char *pre, const char *str) {
	size_t lenpre = strlen(pre),
		   lenstr = strlen(str);
	return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

void append(char*** dest, int* length, char* add) {
	(*length)++;
	size_t new_size = *length * sizeof(char*);
	char** temp = malloc(new_size);
	for(int i = 0; i < (*length) -1; i++)
		temp[i] = (*dest)[i];
	temp[*length-1] = add;

	free(*dest);
	*dest = temp;
}

static int ini_callback(void* user, const char* section, const char* name, const char* value) {
	if (strcmp(section, config_section) != 0) return 1;

	if      (0 == strcmp(name, "music_dir"))    music_dir    = strdup(value);
	else if (0 == strcmp(name, "playlist_dir")) playlist_dir = strdup(value);

	else if (0 == strcmp(name, "target_music_dir"))    target_music_dir    = strdup(value);
	else if (0 == strcmp(name, "target_playlist_dir")) target_playlist_dir = strdup(value);

	else if (0 == strcmp(name, "existing")) existing = strdup(value);

	else if (0 == strcmp(name, "include")) append(&include, &include_length, strdup(value));
	else if (0 == strcmp(name, "exclude")) append(&exclude, &exclude_length, strdup(value));

	else if (0 == strcmp(name, "playlist_prefix"))           playlist_prefix           = strdup(value);
	else if (0 == strcmp(name, "target_playlist_prefix"))    target_playlist_prefix    = strdup(value);
	else if (0 == strcmp(name, "change_playlist_extension")) change_playlist_extension = strdup(value);

	return 1;
}

void exit_err(char* msg) {
	fprintf(stderr, "%s exiting...\n", msg);
	exit(1);
}

void load_config() {
	char* home = getenv("XDG_CONFIG_HOME");
	if(!home) exit_err("$XDG_CONFIG_HOME could not be read!");

	char* config_file_loc = "musicopy/rc.ini";
	char* config_path = join_path(home, config_file_loc);

	if( access(config_path, F_OK) != 0 ) exit_err("Config file could not be read!");

	exclude_length      = 0;
	include_length      = 0;
	existing            = strdup("");
	music_dir           = strdup("");
	playlist_dir        = strdup("");
	target_music_dir    = strdup("");
	target_playlist_dir = strdup("");

	if (ini_parse(config_path, ini_callback, NULL) < 0) exit_err("Can't load configuration file!");
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
		char* new_path = join_path(music_dir, (*listv)[i]);

		free((*listv)[i]);
		(*listv)[i] = new_path;
	}
}

void sha1_file(char* path, unsigned char (*hash)[SHA_DIGEST_LENGTH]) {
	SHA_CTX ctx;
	SHA1_Init(&ctx);

	FILE *file;
	file = fopen(path, "rb");

	if(file == NULL) {
		fprintf(stderr, "couldn't open file \'%s\' in `void sha1_file()`, skipping...\n", path);
		return;
	}

	char buffer[buffer_size];
	size_t size;

	while ((size = fread(buffer, 1, buffer_size, file)))
		SHA1_Update(&ctx, buffer, size);

	fclose(file);
	SHA1_Final(*hash, &ctx);
}

bool duplicate(char* source, char* dest) {
	if (strcmp(existing, "none") == 0) return true;
	else if (strcmp(existing, "lazy") == 0) return false;
	else if (strcmp(existing, "size") == 0) {
		struct stat src_st, dst_st;
		stat(source, &src_st);
		stat(dest, &dst_st);
		return src_st.st_size != dst_st.st_size;
	}
	else if (strcmp(existing, "hash") == 0) {
		unsigned char src_hash[SHA_DIGEST_LENGTH], dst_hash[SHA_DIGEST_LENGTH];
		sha1_file(source, &src_hash);
		sha1_file(dest, &dst_hash);
		return memcmp(src_hash, dst_hash, SHA_DIGEST_LENGTH) != 0;
	}

	return false;
}

void copy(const char* fullpath) {
	int baselen = strlen(music_dir);
	char* basepath = substr(fullpath, baselen);
	char* destpath = join_path(target_music_dir, basepath);

	create_folder_for_file(destpath);

	char* sourcepath = strdup(fullpath);

	bool copy = true;
	if(access(destpath, F_OK) == 0) copy = duplicate(sourcepath, destpath);
	if(copy) {
		int copied = cp(sourcepath, destpath);
		if (copied == 0) printf("%s -> %s\n", sourcepath, destpath);
	}

	free(sourcepath);
}

int music_dir_callback(const char* path, const struct stat *sb, int tflag) {
	if(tflag != FTW_F) return 0;

	if (include_length > 0) {
		bool includes = false;
		for(int i = 0; i < include_length; i++) {
			includes = fnmatch(include[i], path, 0) == 0;
			if(includes) break;
		}
		if (!includes) return 0;
	}

	if (exclude_length > 0)
		for(int i = 0; i < exclude_length; i++)
			if(fnmatch(exclude[i], path, 0) == 0) return 0;

	copy(path);

	return 0;
}

int playlist_dir_callback(const char* path, const struct stat *sb, int tflag) {
	if(tflag != FTW_F) return 0;
	if(dry_run) return 0;

	FILE *source_playlist = fopen(path, "r");

	if(source_playlist == NULL) {
		fprintf(stderr, "couldn't open playlist \'%s\' in `int playlist_dir_callback()`, skipping...\n", path);
		return 0;
	}

	fseek(source_playlist, 0, SEEK_END);
	long max_length = ftell(source_playlist) / sizeof(char) + 1;
	fseek(source_playlist, 0, SEEK_SET);

	char* line = (char*) malloc(max_length * sizeof(char));

	// change base path to target_playlist_dir
	int baselen = strlen(playlist_dir);
	char* basepath = substr(path, baselen);
	char* destpath = join_path(target_playlist_dir, basepath);
	create_folder_for_file(destpath);

	// change file extension
	if(change_playlist_extension != NULL) {
		size_t new_path_size = ((strlen(destpath) + strlen(change_playlist_extension) + 4) * sizeof(char));
		char* new_path = (char*) malloc(new_path_size);
		cwk_path_change_extension(destpath, change_playlist_extension, new_path, new_path_size);

		free(destpath);
		destpath = new_path;
	}

	FILE *dest_playlist = fopen(destpath, "w");

	if(dest_playlist == NULL) {
		fprintf(stderr, "couldn't open playlist \'%s\' in `int playlist_dir_callback()`, skipping...\n", destpath);
		return 0;
	}

	int prefix_len = strlen(playlist_prefix);
	while (fgets(line, max_length, source_playlist) != NULL) {
		char* original_line = strdup(line);

		if (starts_with(playlist_prefix, line))
			sprintf(line, "%s%s", target_playlist_prefix, &original_line[prefix_len]);

		free(original_line);
		fputs(line, dest_playlist);
	}

	fclose(source_playlist);
	fclose(dest_playlist);

	return 0;
}

int main(int argc, char* argv[]) {
	argp_parse(&argp, argc, argv, 0, 0, NULL);

	load_config();

	expandpath(&music_dir);
	expandpath(&playlist_dir);

	expandpath(&target_music_dir);
	expandpath(&target_playlist_dir);

	fix_include_exclude(&include, include_length);
	fix_include_exclude(&exclude, exclude_length);

	ftw(music_dir, music_dir_callback, 0);
	ftw(playlist_dir, playlist_dir_callback, 0);

	return 0;
}
