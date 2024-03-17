#include "parse.h"

#include <flecs.h>
#include <stdlib.h>

const char* parse_c_digit(const char *ptr,int64_t *value_out)
{
    char token[24];
    ptr = ecs_parse_ws_eol(ptr);
    ptr = ecs_parse_digit(ptr, token);
    if (!ptr) {
        goto error;
    }

    *value_out = strtol(token, NULL, 0);

    return ecs_parse_ws_eol(ptr);
error:
    return NULL;
}



char * parse_string(char const *p, char const * needle)
{
	int diff = strncmp(p, needle, strlen(needle));
	if (diff != 0) {
		return NULL;
	}
	return p + strlen(needle);
}

void parse_command(char const * in, int len)
{
	char const * p = in;

	p = parse_string(p, "sub");
	if(p) {
		int64_t value = 0;
		p = parse_c_digit(p, &value);
		if (value) {
			return;
		}
	}



	//eglws_vhd_send_text(vhd, "Subscribing %i:");
	
}



void parse_test()
{
	parse_command("sub 123", 0);
}