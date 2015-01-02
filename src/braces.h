#define MAX_LOOKUP 100
#define MAX_NESTING 30

	
struct runtime {
	FILE *file;
	unsigned short int context_count;
	struct json *context;
};

enum exit_type 
{
	END_OF_FILE,
	CLOSE_TAG
};

void exit_tag(FILE * file);
struct json get_value(struct runtime rt);
void html_puts(char *string);
enum exit_type render(struct runtime rt);
void render_section(struct runtime rt);
enum exit_type render_void(struct runtime rt);
