/* 
 * Copywrite (c) 2014 Thomas Devries <tdevries9@email.davenport.edu>
 * Distributed under the GNU GPL v2. For full terms see the file LICENCE
 * 
 * This utility is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include "json.h"
#define MAX_STRING 2048
#define MAX_ARRAY 40
#define MAX_OBJECT 30

#define ISWHITESPACE(c) \
	(c == ' ' || c == '\t' || c == '\r' || c == '\n')
#define JSON_ERROR(code) \
	(struct json){.type = JSON_UNDEFINED, .val = {.error = code}}

enum json_error {
	UNEXPECTED_EOF,
	MALFORMED_PRIMATIVE,
	OVERSIZED_ARRAY,
	OVERSIZED_STRING,
	MALFORMED_ARRAY,
	UNKNOWN_ESCAPE_SEQUENCE,
	INVALID_OBJECT_KEY,
	MISSING_COLEN,
	INVALID_OBJECT_TERMINATOR,
	UNKNOWN_TYPE,
	INVALID_UTF_SEQUENCE
};

char *error_strings[] = {
	[UNEXPECTED_EOF] = "Unexpected end of file",
	[MALFORMED_PRIMATIVE] = "Unknown or malformed primative",
	[OVERSIZED_ARRAY] = "Array is to large max",
	[OVERSIZED_STRING] = "String is to large",
	[MALFORMED_ARRAY] = "Expected ']'",
	[UNKNOWN_ESCAPE_SEQUENCE] = "Escape sequence is unknown",
	[INVALID_OBJECT_KEY] = "Object keys must be strings",
	[MISSING_COLEN] = "Key value pares must be split with ':'",
	[INVALID_OBJECT_TERMINATOR] = "Objects must be closed with '}'",
	[UNKNOWN_TYPE] = "Unknown/unsuported type",
	[INVALID_UTF_SEQUENCE] = "Unicode is not supported yet"
};

struct json json_parse(FILE *file) {
	char c;
	do c = getc(file);
	while (ISWHITESPACE(c));
	struct json ret;
	
	switch (c) {
		
		case EOF:
			ret = JSON_ERROR(UNEXPECTED_EOF);
			break;
			
		default:
			ret = JSON_ERROR(UNKNOWN_TYPE);
			break;
			
		case 't':
			if (getc(file) == 'r' &&
				getc(file) == 'u' &&
				getc(file) == 'e') {
				ret.type = JSON_TRUE;
			} else {
				ret = JSON_ERROR(MALFORMED_PRIMATIVE);
			}
			break;
			
		case 'f':
			if (getc(file) == 'a' &&
				getc(file) == 'l' &&
				getc(file) == 's' &&
				getc(file) == 'e') {
				ret.type = JSON_FALSE;
			} else {
				ret = JSON_ERROR(MALFORMED_PRIMATIVE);
			}
			break;
		
		case 'n':
			if (getc(file) == 'u' &&
				getc(file) == 'l' &&
				getc(file) == 'l') {
				ret.type = JSON_NULL;
			} else {
				ret =  JSON_ERROR(MALFORMED_PRIMATIVE);
			}
			break;
		
		case '+': case '-': case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7': case '8': case '9':
			ungetc(c, file);
			fscanf(file, "%lf", &ret.val.number);
			ret.type = JSON_NUMBER;
			break;
			
		case '"':
		/* string parser */
		{
			char *str = malloc(MAX_STRING);
			unsigned int index = 0;
			while ( (c = getc(file)) != '"') {
				if (c == EOF) {
					ret = JSON_ERROR(UNEXPECTED_EOF);
					goto string_error;
				}
				if (c == '\\') {
					c = getc(file);
					switch (c) {
						case EOF: 
							ret = JSON_ERROR(UNEXPECTED_EOF);
							goto string_error;
						
						case 'b': c = '\b'; break;
						case 'f': c = '\f'; break;
						case 'n': c = '\n'; break;
						case 'r': c = '\r'; break;
						case 't': c = '\t'; break;
						
						case 'u': 
							ret = JSON_ERROR(INVALID_UTF_SEQUENCE);
							goto string_error;
					}
				}
				
				str[index++] = c;
				if (index == MAX_STRING) {
					ret = JSON_ERROR(OVERSIZED_STRING);
					goto string_error;
				}
				
			}
			
			str[index++] = '\0';
			str = realloc(str, index);
			ret = (struct json){JSON_STRING, index - 1, {.string=str}};
			break;
			
			string_error:free(str);
			break;
		}
		case '[':
		/* array parser */
		{
			unsigned int index = 0;
			do c = getc(file);
			while (ISWHITESPACE(c));
			if (c == ']') {
				ret.type = JSON_ARRAY;
				ret.val.array = NULL;
				ret.size = 0;
				break;
			}
			ungetc(c, file);
			struct json *array = malloc(MAX_ARRAY * sizeof(struct json));
			do {
				if (index == MAX_ARRAY) {
					ret = JSON_ERROR(OVERSIZED_ARRAY);
					goto array_error;
				}
				array[index] = json_parse(file);
				if (array[index].type == JSON_UNDEFINED) {
					ret = array[index];
					index++;
					goto array_error;
				}
				do c = getc(file);
				while (ISWHITESPACE(c));
				index ++;
			} while (c == ',');
			if (c != ']') {
				ret = JSON_ERROR(MALFORMED_ARRAY);
				goto array_error;
			}
			array = realloc(array, sizeof(struct json) * index);
			ret.type = JSON_ARRAY;
			ret.val.array = array;
			ret.size = index;
			break;
			array_error:free(array);
			break;
		}
		
		case '{':
		/* object/hashmap parser */
		{
			unsigned int index = 0;
			struct json key;
			do c = getc(file);
			while (ISWHITESPACE(c));
			if (c == '}') {
				ret.type = JSON_OBJECT;
				ret.val.object = NULL;
				ret.size = 0;
				break;
			}
			ungetc(c, file);
			struct json_object *object = 
				malloc(MAX_OBJECT * sizeof(struct json_object));
			do {
				key = json_parse(file);
				if (key.type != JSON_STRING) {
					index++;
					ret = JSON_ERROR(INVALID_OBJECT_KEY);
					goto object_error;
				}
				object[index].key = key.val.string;
				c = getc(file);
				if (c != ':') {
					ret = JSON_ERROR(MISSING_COLEN);
					index++;
					goto object_error;
				}
				object[index].value = json_parse(file);
				if (object[index].value.type == JSON_UNDEFINED) {
					ret = object[index].value;
					index++;
					goto object_error;
				}
				do c = getc(file);
				while (ISWHITESPACE(c));
				index++;
			} while (c == ',');
			if (c != '}') {
				ret = JSON_ERROR(INVALID_OBJECT_TERMINATOR);
				goto object_error;
			}
			object = realloc(object, index * sizeof(struct json_object));
			ret.type = JSON_OBJECT;
			ret.val.object = object;
			ret.size = index;
			break;
			object_error: free(object);
			break;
		}
	}
	return ret;
}


struct json json_lookup(struct json j, char *lookup)
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
		key = j.val.object[i].key;
		index = 0;
		for (;;) {
			if (key[index] == '\0') {
				if (lookup[index] == '\0')
					return j.value.object[i].value;
				if (lookup[index] == '.')
					return json_object_lookup(
						j.val.object[i].value,
						lookup + index + 1);
			}
			if (key[index] != lookup[index])
				break;
			index++;
		}
	}
	return ret;
}
