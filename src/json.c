#include "json.h"

#define ISWHITESPACE(c) \
	(c == ' ' || c == '\n' || c == '\t' || c == '\r')
#define JSON_THROW(condition, code) \
	if (condition) { \
		fputs("JSON ERROR: " code "\n", stderr);  \
		exit(200); \
	}

char c;
void static parse_any(struct json *ret);
void static parse_string(char **ret);

void static parse_object(
	struct json_object **object, unsigned int *size)
{

	*size = 0;
	unsigned int physical_size = JSON_OBJECT_CHUNK;
	*object = malloc(JSON_OBJECT_CHUNK * sizeof(struct json_object));
	do c = getc(stdin);
	while (ISWHITESPACE(c));
	if (c == '}') {
		*object = NULL;
		c = getc(stdin);
		return;
	}
	for (;;) {
		JSON_THROW(c == EOF, "Unexpected EOF");
		while (ISWHITESPACE(c))
			c = getc(stdin);
		JSON_THROW(c != '"', "Object keys must be strings");
		*size +=1;
		parse_string(&((*object)[*size-1].key));
		while (ISWHITESPACE(c))
			c = getc(stdin);
		JSON_THROW(c != ':', "Expected ':'");
		do c = getc(stdin);
		while (ISWHITESPACE(c));
		parse_any(&((*object)[*size-1].value));
		while (ISWHITESPACE(c))
			c = getc(stdin);
		if (c != ',') break;
		c = getc(stdin);
		if (*size > physical_size) {
			physical_size += JSON_OBJECT_CHUNK;
			*object = realloc(*object,
				physical_size * sizeof(struct json_object));
		}
	}
	JSON_THROW(c != '}', "Unterminated object");
	*object = realloc(*object, *size * sizeof(struct json_object));
	c = getc(stdin);
}

void static parse_array(struct json **array, unsigned int *size)
{
	*size = 0;
	unsigned int physical_size = JSON_ARRAY_CHUNK;
	*array = malloc(JSON_ARRAY_CHUNK * sizeof(struct json));
	do c = getc(stdin);
	while (ISWHITESPACE(c));
	if (c == ']') {
		*array = NULL;
		c = getc(stdin);
		return;
	}
	for(;;) {
		JSON_THROW(c == EOF, "Unexpected EOF");
		while(ISWHITESPACE(c))
			c = getc(stdin);
		(*size)++;
		parse_any((*array + *size - 1));
		
		while(ISWHITESPACE(c))
			c = getc(stdin);
		if (c != ',') break;
		if ((*size) > physical_size) {
			physical_size += JSON_ARRAY_CHUNK;
			*array = realloc( *array,
				physical_size * sizeof(struct json));
		}
		c = getc(stdin);
	}
	JSON_THROW(c != ']', "Unterminated array");
	c = getc(stdin);
	*array = realloc(*array, *size * sizeof(struct json));
}

void parse_string(char **ret) 
{
	unsigned int size = JSON_STRING_CHUNK;
	char *str = (*ret) = malloc(size);
	unsigned int index = 0;
	while ( (c = getc(stdin)) != '"') {
		JSON_THROW(c == EOF, "Unexpected EOF");
		if (c == '\\') {
			c = getc(stdin);
			switch (c) {
			case '\\':case '\'': case '"': break;
			case 't': c = '\t'; break;
			case 'b': c = '\b'; break;
			case 'f': c = '\f'; break;
			case 'n': c = '\n'; break;
			case 'r': c = '\r'; break;
			case EOF:
				JSON_THROW(1, "Unexpected EOF");
				break;
			default: 
				JSON_THROW(1, "Unknown escape sequence");
				break;
			}
		}
		str[index++] = c;
		if (index >= size) {
			size += JSON_STRING_CHUNK;
			str = (*ret) = realloc(str, size);
		}
	}
	c = getc(stdin);
	str[index++] = '\0';
	str = (*ret) = realloc(str, index);
	return;
}

void static parse_any(struct json *ret) 
{
	while (ISWHITESPACE(c))
		c = getc(stdin);
	switch (c) {
		case '"':
			ret->type = JSON_STRING;
			parse_string(&(ret->value.string));
			break;
		case '[':
			ret->type = JSON_ARRAY;
			parse_array(&ret->value.array, &ret->size);
			break;
		case '{':
			ret->type = JSON_OBJECT;
			parse_object(&ret->value.object, &ret->size);
			break;
		case 't':
			JSON_THROW(getc(stdin) != 'r', "Unknown primative");
			JSON_THROW(getc(stdin) != 'u', "Unknown primative");
			JSON_THROW(getc(stdin) != 'e', "Unknown primative");
			c = getc(stdin);
			ret->type = JSON_TRUE;
			break;
		case 'f':
			JSON_THROW(getc(stdin) != 'a', "Unknown primative");
			JSON_THROW(getc(stdin) != 'l', "Unknown primative");
			JSON_THROW(getc(stdin) != 's', "Unknown primative");
			JSON_THROW(getc(stdin) != 'e', "Unknown primative");
			c = getc(stdin);
			ret->type = JSON_FALSE;
			break;
		case 'n':
			JSON_THROW(getc(stdin) != 'u', "Unknown primative");
			JSON_THROW(getc(stdin) != 'l', "Unknown primative");
			JSON_THROW(getc(stdin) != 'l', "Unknown primative");
			c = getc(stdin);
			ret->type = JSON_NULL;
			break;
		case EOF:
			JSON_THROW(1, "Unexpected EOF");
		default: JSON_THROW(1, "Unknown type");
	}
}

struct json json_parse() 
{
	struct json ret;
	c = ' ';
	parse_any(&ret);
	return ret;
}


struct json json_object_lookup(struct json j, char *lookup) 
{
	char *key;
	unsigned int index;
	unsigned int i;
	struct json ret;
	ret.type = JSON_UNDEFINED;
	if (lookup[0] == '.' && lookup[1] == '\0')
		return j;
	if (j.type != JSON_OBJECT)
		return ret;
	for (i = 0; i < j.size; i++) {
		key = j.value.object[i].key;
		index = 0;
		for (;;) {
			if (key[index] == '\0') {
				if (lookup[index] == '\0')
					return j.value.object[i].value;
				if (lookup[index] == '.')
					return json_object_lookup(
						j.value.object[i].value,
						lookup + index + 1);
			}
			if (key[index] != lookup[index])
				break;
			index++;
		}
	}
	return ret;
}


