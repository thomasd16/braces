#include <stdio.h>
struct json;
struct json_object;
struct json json_parse(FILE *file);
enum json_type;
#define JSON_BOOL(data) \
	(data.type == JSON_UNDEFINED? 0:\
	data.type == JSON_FALSE?     0:\
	data.type == JSON_NULL?      0:\
	data.type == JSON_TRUE?      1:\
	data.type == JSON_STRING? data.val.string[0] != 0:\
	data.type == JSON_ARRAY? data.size != 0:\
	data.type == JSON_OBJECT? 1 :\
	0)
	
#define JSON_STRINGIFY(data) \
	(data.type == JSON_UNDEFINED? "undefined" :\
	data.type == JSON_NULL? "null":\
	data.type == JSON_TRUE? "true":\
	data.type == JSON_FALSE? "false":\
	data.type == JSON_STRINGIFY? data.val.string:\
	"")
	
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
