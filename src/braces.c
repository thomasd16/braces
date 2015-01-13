#include <stdio.h>
#include "json.h"
#include "braces.h"

#define ISWHITESPACE(c) \
	(c == ' ' || c == '\n' || c == '\t' || c == '\r')
	
#define BRACES_THROW(condition, code) \
	if (condition) {\
		fputs(code "\n", stderr);\
		exit(1);\
	}
	
#define CHKEOF(c) BRACES_THROW(c == EOF, "Unexpected EOF")

char c;

void exit_tag(FILE * file) {
	for (;;) {
		do c = getc(file);
		while ( c != '}' && c != EOF);
		CHKEOF(c);
		c = getc(file);
		CHKEOF(c);
		if (c == '}') return;
	}
}

struct json get_value(struct runtime rt)
{
	char lookup[MAX_LOOKUP];
	struct json ret;
	unsigned short int index = 0;
	while( (c = getc(rt.file)) != '}' && c != EOF) {
		if(ISWHITESPACE(c)) continue;
		lookup[index++] = c;
		BRACES_THROW(index >= MAX_LOOKUP, "Lookup is to large");
	}
	lookup[index] = '\0';
	CHKEOF(c);
	c = getc(rt.file);
	BRACES_THROW(c != '}', "Incorrectly terminated tag");
	index = rt.context_count;
	while (index--) {
		ret = json_object_lookup(rt.context[index], lookup);
		if (ret.type != JSON_UNDEFINED)
			return ret;
	}
	return ret;
}

void html_puts(char *string) 
{
	unsigned int i;
	for (i = 0; string[i] != '\0'; i++) {
		switch (string[i]) {
			case '&' :  fputs("&amp;", stdout);  break;
			case '\"':  fputs("&quot;", stdout); break;
			case '\'':  fputs("&apos;", stdout); break;
			case '>' :  fputs("&lt;", stdout);   break;
			case '<' :  fputs("&gt;", stdout);   break;
			default  :  putchar(string[i]);      break;
		}
	}
}

void render_inverse(struct runtime rt)
{
	struct json data = get_value(rt);
	if (!JSON_BOOL(data))
		render(rt);
	else 
		render_void(rt);
}
void render_section(struct runtime rt) 
{
		struct json data = get_value(rt);
		if (!JSON_BOOL(data)) {
			render_void(rt);
			return;
		}
		if (data.type == JSON_OBJECT) {
			rt.context[rt.context_count++] = data;
			render(rt);
			rt.context_count--;
			return;
		}
		if (data.type == JSON_ARRAY) {
			unsigned short int i;
			unsigned long int f_pos = ftell(rt.file);
			struct json *new_context =
				&rt.context[rt.context_count];
			rt.context_count++;
			for (i = 0; i < data.size; i++) {
				*new_context = data.value.array[i];
				render(rt);
				if (i < data.size - 1) 
					fseek(rt.file, f_pos, SEEK_SET);
			}
			rt.context_count--;
			return;	
		}
		render(rt);
}

enum exit_type render_void(struct runtime rt) 
{
	unsigned short int nesting_depth = 0;
	for (;;) {
		do c = getc(rt.file);
		while ( c != '{' && c != EOF);
		if (c == EOF) 
			return END_OF_FILE;
		c = getc(rt.file);
		if (c != '{') continue;
		do c = getc(rt.file);
		while (ISWHITESPACE(c));
		switch (c) 
		{
			case '#':
				nesting_depth++;
				exit_tag(rt.file);
				break;
			case '^':
				nesting_depth++;
				exit_tag(rt.file);
				break;
			case '/':
				exit_tag(rt.file);
				if (!nesting_depth--)
					return CLOSE_TAG;
				break;
			case '{':
				exit_tag(rt.file);
				c = getc(rt.file);
				BRACES_THROW(c != '}', "Incorrectly terminated tag");
				break;
			default:
				exit_tag(rt.file);
				break;
		}
	}
}

enum exit_type render(struct runtime rt)
{
	struct json output;
	for (;;) {
		while ( (c = getc(rt.file)) != '{' && c != EOF)
			putchar(c);
		if (c == EOF) 
			return END_OF_FILE;
		if ( (c = getc(rt.file)) != '{') {
			putchar('{');
			putchar(c);
		} else {
			do c = getc(rt.file);
			while (ISWHITESPACE(c));
			CHKEOF(c);
			switch (c) {
			case '!':
				exit_tag(rt.file);
				break;
			case '&': 
				output = get_value(rt);
				fputs(JSON_STRINGIFY(output), stdout);
				break;
			case '{':
				output = get_value(rt);
				fputs(JSON_STRINGIFY(output), stdout);
				c = getc(rt.file);
				BRACES_THROW(c != '}', "Incorrectly terminated tag");
				break;
			case '#':
				render_section(rt);
				break;
			case '^':
				render_inverse(rt);
				break;
			case '/':
				exit_tag(rt.file);
				return CLOSE_TAG;
			default:
				fseek(rt.file, -1, SEEK_CUR);	
				output = get_value(rt);
				html_puts(JSON_STRINGIFY(output));
				break;
			}
		}
	}
}

int main (int argc, char *argv[])
{
	struct runtime rt;
	struct json context[MAX_NESTING];
	BRACES_THROW( argc < 2, "No file specified");
	rt.context = context;
	rt.context[0] = json_parse(stdin);
	rt.context_count = 1;
	rt.file = fopen(argv[1], "r");
	BRACES_THROW(rt.file == NULL, "Failed to open file");
	enum exit_type ret =  render(rt);
	BRACES_THROW(ret != END_OF_FILE, "You had a nesting error");
	return 0;
}
