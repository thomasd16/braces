#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#define JSON_STRING_CHUNK 2048
#define JSON_OBJECT_CHUNK 30
#define JSON_ARRAY_CHUNK 60
#define JSON_BOOL(j) !!( \
	j.type == JSON_TRUE? 1 : \
	(j.type == JSON_OBJECT || j.type == JSON_ARRAY)? j.size : \
	j.type == JSON_STRING? j.value.string[0] : \
	0) 
#define JSON_STRINGIFY(j) (\
	j.type == JSON_STRING? j.value.string: \
	j.type == JSON_TRUE? "true": \
	j.type == JSON_FALSE? "false": \
	j.type == JSON_NULL? "null": \
	"")

typedef struct json json;
struct object;
enum json_type {
	JSON_UNDEFINED = 0,
	JSON_NULL,
	JSON_TRUE,
	JSON_FALSE,
	JSON_ERROR,
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT
};

struct json {
	enum json_type type;
	unsigned int size;
	union {
		json *array;
		char *string;
		struct json_object *object;
	} value;
};

struct json_object {
	char *key;
	struct json value;	
};



struct json json_parse();
struct json json_object_lookup(struct json origin, char *lookup);
