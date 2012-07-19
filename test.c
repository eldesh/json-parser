#define _CRT_SECURE_NO_WARNINGS
#if defined __cplusplus
# include <cstdio>
# include <cstdlib>
# include <cstdint>
# include <cstring>
# include <cassert>
#else
# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <assert.h>
#endif
#include <error.h>

#include "json.h"

#if defined _WIN32
#  define SEP "\\"
#else
#  define SEP "/"
#endif

#if !defined __cplusplus
typedef enum { false=0, true } bool;
#endif

static char const * valid_files[] = {
	"valid-0000.json",	"valid-0001.json",
	"valid-0002.json",	"valid-0003.json"
};
static int const valid_file_size = sizeof(valid_files)/sizeof(valid_files[0]);

static char const * invalid_files[] = {
	"invalid-0000.json",	"invalid-0001.json",
	"invalid-0002.json",	"invalid-0003.json",
	"invalid-0004.json",	"invalid-0005.json"
};
static int const invalid_file_size = sizeof(invalid_files)/sizeof(invalid_files[0]);

static int file_size(FILE * fp) {
	int curr = ftell(fp);
	int size=0;
	fseek(fp, 0   , SEEK_END); // begin of fp
	size = ftell(fp);
	fseek(fp, curr, SEEK_SET); // recovery offset
	if (size==-1)
		fprintf(stderr, "%s\n", strerror(errno));
	return size;
}

// read all contents of given file
char * read_file (char const * file) {
	FILE * fp = fopen(file, "r");
	if (fp) {
		int size = file_size(fp);
		char * buf = (char*)malloc(size > 1 ? size : 1); // avoid undefined behaviour
		if (buf) {
			size_t rsize = fread(buf, 1, size, fp);
			if (rsize==-1) {
				fprintf(stderr, "%s\n", strerror(errno));
				free(buf);
			} else {
				buf[rsize]='\0';
				return buf;
			}
		}
	}
	return NULL;
}

// return true if parsing given file is successed
bool test_json_parse_file(char const * dir, char const * filename) {
	char path[256];
	char * buf=NULL;
	sprintf(path, "%s" SEP "%s", dir, filename);
	buf = read_file(path);
	if (buf) {
		json_value * v = json_parse(buf);
		if (v) {
			json_value_free(v);
			return true;
		} else
			return false;
	} else {
		fprintf(stderr, "read file <%s> failed\n", path);
		return false;
	}
}

int main () {
	int i;
	for (i=0; i<valid_file_size; ++i) {
		// require success
		printf("test json_parse(%s) is %s\n", valid_files[i]
					, test_json_parse_file("tests", valid_files[i]) ? "pass" : "fail");
	}
	for (i=0; i<invalid_file_size; ++i) {
		// require fail
		printf("test json_parse(%s) is %s\n", invalid_files[i]
					, !test_json_parse_file("tests", invalid_files[i]) ? "pass" : "fail");
	}
	return 0;
}
