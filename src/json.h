#include <stdio.h>
struct json;
struct json_object;
struct json json_parse(FILE *file);
enum json_type;

enum json_type {
	JSON_UNDEFINED = 0,
	JSON_FALSE,
	JSON_TRUE,
	JSON_NULL,
	JSON_STRING,
	JSON_NUMBER,
	JSON_ARRAY,
	JSON_OBJECT
};

struct json {
	enum json_type type;
	unsigned short int size;
	union {
		struct json *array;
		struct json_object *object;
		char *string;
		double number;
		int error;
	} val;
};

struct json_object {
	char *key;
	struct json value;
};
