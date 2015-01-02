#include <stdio.h>
/* file for testing not for final product */
void dump_any(struct json obj);
void dump_string(char * c) {
	putchar('"');
	while (*c != '\0') {
		switch (*c) {
		case '\b': putchar('\\'); putchar('b'); break;
		case '\f': putchar('\\'); putchar('f'); break;
		case '\n': putchar('\\'); putchar('n'); break;
		case '\r': putchar('\\'); putchar('r'); break;
		case '\t': putchar('\\'); putchar('t'); break;
		case '\'': case '\\': case '\"': case '/':
			putchar('\\');putchar(*c); 
			break;
		default:putchar(*c);break;
		}
		c++;
	}
	putchar('"');
}

void dump_object(struct json obj) {
	int i;
	struct json_object item;
	putchar('{');
	if (obj.size == 0) {
		putchar('}');
		return;
	}
	for (i = 0; i < obj.size; i++) {
		item = obj.value.object[i];
		dump_string(item.key);
		putchar(':');
		dump_any(item.value);
		if ( (i + 1) < obj.size) {
			putchar(',');
		}
	}
	putchar('}');
}

void dump_array(struct json arr) {
	int i;
	putchar('[');
	if (arr.size == 0) {
		putchar(']');
		return;
	}
	for (i = 0; i < arr.size; i++) {
		dump_any(arr.value.array[i]);
		if ( (i + 1) < arr.size) {
			putchar(',');
		}
	}
	putchar(']');
}
void dump_any(struct json obj) {
	switch (obj.type) {
	case JSON_STRING: dump_string(obj.value.string); break;
	case JSON_OBJECT: dump_object(obj); break;
	case JSON_ARRAY: dump_array(obj); break;
	case JSON_TRUE: fputs("true", stdout); break;
	case JSON_FALSE: fputs("false", stdout); break;
	case JSON_NULL: fputs("null", stdout); break;
	case JSON_UNDEFINED: puts("ERROR: undefined cannot be written"); break;
	case JSON_ERROR: fputs("ERROR: ", stdout); puts(obj.value.string); break;
	}
}
