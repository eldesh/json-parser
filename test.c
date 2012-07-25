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
		return json_value_free(v), v!=NULL;
	} else {
		fprintf(stderr, "read file <%s> failed\n", path);
		return false;
	}
}

bool test_json_parse (char const * s) {
	json_value * v = json_parse(s);
	json_value_free(v);
	if (v)
		printf("test parse (%s) passed\n", s);
	else
		printf("test parse (%s) failed\n", s);
	return v!=NULL;
}

bool test_find_json_object(void) {
	json_value * v = json_parse("{\"foo\":314}");
	bool r = v && find_json_object(v, "foo") ;
	return false;
}

#if defined _WIN32
#  define __func__ __FUNCTION__
#endif

#define TEST_JSON_VALUE_CMP(cmp, lhs, rhs)                             \
		do {                                                           \
			json_value * lhs_ = (lhs);                                 \
			json_value * rhs_ = (rhs);                                 \
			printf("%20s:%5d@%-10s: ", __FILE__, __LINE__, __func__);  \
			if (cmp(lhs_, rhs_)) {                                     \
				printf("passed\n");                                    \
			} else {                                                   \
				json_value_dump(stdout, lhs);                          \
				printf(" != ");                                        \
				json_value_dump(stdout, rhs);						   \
				printf("\n");                                          \
			}                                                          \
			json_value_free(lhs_);                                     \
			json_value_free(rhs_);                                     \
		} while (0)

#define TEST_JSON_VALUE_EQ(lhs, rhs)     TEST_JSON_VALUE_CMP( json_value_equal, lhs, rhs)
#define TEST_JSON_VALUE_NOT_EQ(lhs, rhs) TEST_JSON_VALUE_CMP(!json_value_equal, lhs, rhs)
#define TEST_JSON_TYPE_EQ(lhs, rhs)      TEST_JSON_VALUE_CMP(  json_type_equal, lhs, rhs)
#define TEST_JSON_TYPE_NOT_EQ(lhs, rhs)  TEST_JSON_VALUE_CMP( !json_type_equal, lhs, rhs)

void test_json_value_equal(void) {
	json_value * lhs = NULL;
	json_value * rhs = NULL;
	// expect equal
	TEST_JSON_VALUE_EQ(NULL, NULL);
	TEST_JSON_VALUE_EQ(json_parse("314"), json_parse("314"));
	TEST_JSON_VALUE_EQ(json_parse("null"), json_parse("null"));
	TEST_JSON_VALUE_EQ(json_parse("{}"), json_parse("{}"));
	TEST_JSON_VALUE_EQ(json_parse("[]"), json_parse("[]"));
	TEST_JSON_VALUE_EQ(json_parse("[1,2,3]"), json_parse("[1,2,3]"));
	TEST_JSON_VALUE_EQ(json_parse("[]"), json_parse("[]"));
	TEST_JSON_VALUE_EQ(json_parse("\"\""), json_parse("\"\""));
	TEST_JSON_VALUE_EQ(json_parse("\"foo\""), json_parse("\"foo\""));
	TEST_JSON_VALUE_EQ(json_parse("\"foo bar bazz\""), json_parse("\"foo bar bazz\""));
	TEST_JSON_VALUE_EQ(json_parse("[7298,\"\",{}]")  , json_parse("[7298,\"\",{}]"));
	TEST_JSON_VALUE_EQ(json_parse("{\"foo\":123}"), json_parse("{\"foo\":123}"));
	TEST_JSON_VALUE_EQ(json_parse("{\"test.c\":256, \"json.c\":512}"), json_parse("{\"json.c\":512, \"test.c\":256}"));
	TEST_JSON_VALUE_EQ(json_parse("{\"alice\":[12,\"white rabbit\"] \
								,   \"knight\":[\"KJQ\" \
												, \"as the Knight fell heavily on the top of his head exactly in the path where Alice was walking.\"]}")
					,  json_parse("{\"knight\":[\"KJQ\" \
												, \"as the Knight fell heavily on the top of his head exactly in the path where Alice was walking.\"] \
								,   \"alice\":[12,\"white rabbit\"]}"));
	TEST_JSON_VALUE_EQ(json_parse("{\"define\": [\"fib\", [\"lambda\", \"x\" \
															, [\"cond\", \
																[[\"eq\", \"x\", 0], 1], \
																[[\"eq\", \"x\", 1], 1], \
																[[\"other\", [\"+\", [\"fib\", [\"-\", \"x\", 1]], [\"fib\", [\"-\", \"x\", 2]]]]]]]]}")
					,  json_parse("{\"define\": [\"fib\", [\"lambda\", \"x\" \
															, [\"cond\", \
																[[\"eq\", \"x\", 0], 1], \
																[[\"eq\", \"x\", 1], 1], \
																[[\"other\", [\"+\", [\"fib\", [\"-\", \"x\", 1]], [\"fib\", [\"-\", \"x\", 2]]]]]]]]}"));

	// expect *not* equal
	TEST_JSON_VALUE_NOT_EQ(json_parse("[]"), json_parse("{}"));
	TEST_JSON_VALUE_NOT_EQ(json_parse("1.42"), json_parse("1.42"));
	TEST_JSON_VALUE_NOT_EQ(json_parse("\"foo bar bazz\""), json_parse("\"foo bar bazz \""));
	TEST_JSON_VALUE_NOT_EQ(json_parse("\" foo bar bazz\""), json_parse("\"foo bar bazz\""));
	TEST_JSON_VALUE_NOT_EQ(json_parse("[20, 30, 40]"), json_parse("[30, 40, 20]"));
	TEST_JSON_VALUE_NOT_EQ(json_parse("{\"define\": [\"fib\", [\"lambda\", \"x\" \
															, [\"cond\", \
																[[\"eq\", \"x\", 0], 1], \
																[[\"eq\", \"x\", 1], 1], \
																[[\"other\", [\"+\", [\"fib\", [\"-\", \"x\", 1]], [\"fib\", [\"-\", \"x\", 2]]]]]]]]}")
						,  json_parse("{\"define\": [\"fib\", [\"lambda\", \"y\" \
															, [\"cond\", \
																[[\"eq\", \"y\", 0], 1], \
																[[\"eq\", \"y\", 1], 1], \
																[[\"other\", [\"+\", [\"fib\", [\"-\", \"y\", 1]], [\"fib\", [\"-\", \"y\", 2]]]]]]]]}"));
}

void test_json_type_equal (void) {
	json_value * lhs = NULL;
	json_value * rhs = NULL;
	// expect equal
	TEST_JSON_TYPE_EQ(NULL, NULL);
	TEST_JSON_TYPE_EQ(json_parse("314"), json_parse("314"));
	TEST_JSON_TYPE_EQ(json_parse("null"), json_parse("null"));
	TEST_JSON_TYPE_EQ(json_parse("{}"), json_parse("{}"));
	TEST_JSON_TYPE_EQ(json_parse("[]"), json_parse("[]"));
	TEST_JSON_TYPE_EQ(json_parse("[1,2,3]"), json_parse("[1,2,3]"));
	TEST_JSON_TYPE_EQ(json_parse("[]"), json_parse("[]"));
	TEST_JSON_TYPE_EQ(json_parse("\"\""), json_parse("\"\""));
	TEST_JSON_TYPE_EQ(json_parse("\"foo\""), json_parse("\"foo\""));
	TEST_JSON_TYPE_EQ(json_parse("\"foo bar bazz\""), json_parse("\"foo bar bazz\""));
	TEST_JSON_TYPE_EQ(json_parse("[7298,\"\",{}]")  , json_parse("[7298,\"\",{}]"));
	TEST_JSON_TYPE_EQ(json_parse("{\"foo\":123}"), json_parse("{\"foo\":123}"));
	TEST_JSON_TYPE_EQ(json_parse("{\"test.c\":256, \"json.c\":512}"), json_parse("{\"json.c\":512, \"test.c\":256}"));
	TEST_JSON_TYPE_EQ(json_parse("{\"alice\":[12,\"white rabbit\"] \
								,   \"knight\":[\"KJQ\" \
												, \"as the Knight fell heavily on the top of his head exactly in the path where Alice was walking.\"]}")
					,  json_parse("{\"knight\":[\"KJQ\" \
												, \"as the Knight fell heavily on the top of his head exactly in the path where Alice was walking.\"] \
								,   \"alice\":[12,\"white rabbit\"]}"));
	TEST_JSON_TYPE_EQ(json_parse("{\"define\": [\"fib\", [\"lambda\", \"x\" \
															, [\"cond\", \
																[[\"eq\", \"x\", 0], 1], \
																[[\"eq\", \"x\", 1], 1], \
																[[\"other\", [\"+\", [\"fib\", [\"-\", \"x\", 1]], [\"fib\", [\"-\", \"x\", 2]]]]]]]]}")
					,  json_parse("{\"define\": [\"fib\", [\"lambda\", \"x\" \
															, [\"cond\", \
																[[\"eq\", \"x\", 0], 1], \
																[[\"eq\", \"x\", 1], 1], \
																[[\"other\", [\"+\", [\"fib\", [\"-\", \"x\", 1]], [\"fib\", [\"-\", \"x\", 2]]]]]]]]}"));

	// expect *not* equal
	TEST_JSON_TYPE_NOT_EQ(json_parse("[]"), json_parse("{}"));
	TEST_JSON_TYPE_NOT_EQ(json_parse("1.42"), json_parse("[1.42]"));
	TEST_JSON_TYPE_NOT_EQ(json_parse("\"foo bar bazz\""), json_parse("{\"\":\"foo bar bazz \"}"));
	TEST_JSON_TYPE_NOT_EQ(json_parse("[\"foo\", \"bar\", \"bazz\"]"), json_parse("[\"foo\" \"bazz\", \"bar\"]"));
	TEST_JSON_TYPE_NOT_EQ(json_parse("[20, 30, 40]"), json_parse("[30, [40, [20]]]"));
	TEST_JSON_TYPE_NOT_EQ(json_parse("{\"define\": [\"fib\", [\"lambda\", \"x\" \
															, {\"cond\": [\
																[[\"eq\", \"x\", 0], 1], \
																[[\"eq\", \"x\", 1], 1], \
																[[\"other\", [\"+\", [\"fib\", [\"-\", \"x\", 1]], [\"fib\", [\"-\", \"x\", 2]]]]]]}]]}")
						,  json_parse("{\"define\": [\"fib\", [\"lambda\", \"y\" \
															, [\"cond\", \
																[[\"eq\", \"y\", 0], 1], \
																[[\"eq\", \"y\", 1], 1], \
																[[\"other\", [\"+\", [\"fib\", [\"-\", \"y\", 1]], [\"fib\", [\"-\", \"y\", 2]]]]]]]]}"));
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
	test_json_value_equal();
	test_json_type_equal ();
	return 0;
}
