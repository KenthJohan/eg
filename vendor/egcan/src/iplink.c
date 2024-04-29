#include "iplink.h"
#include "json.h"
#include <stdio.h>
#include <flecs.h>
#include <stdlib.h>

static char const *popen_to_string(char const *command)
{
	FILE *fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failed to run command\n");
		return NULL;
	}

	char buf[1035];
	ecs_strbuf_t b = ECS_STRBUF_INIT;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		ecs_strbuf_appendstr(&b, buf);
	}
	pclose(fp);
	return ecs_strbuf_get(&b);
}

#define TOK_COUNT 1024


static jsmntok_t * parse1(char const *json, jsmntok_t * u, iplink_info_t * out)
{
	if (u->type != JSMN_OBJECT) {
		printf("Array expected\n");
	}
	u++;
	json_parse_value(json, u, (char const *[]){"ifindex", NULL}, &out->ifindex, JSON_TYPE_INT);
	json_parse_string(json, u, (char const *[]){"ifname", NULL}, out->ifname, 128);
	json_parse_value(json, u, (char const *[]){"mtu", NULL}, &out->mtu, JSON_TYPE_INT);
	json_parse_value(json, u, (char const *[]){"linkinfo", "info_data", "bittiming", "bitrate", NULL}, &out->can_bitrate, JSON_TYPE_INT);
	while(1) {
		if (u->parent == 0) {
			break;
		}
		u++;
	}
	return u;
}

int iplink_parse(iplink_info_t out[], int out_length)
{
	int n = 0;
	char const *json = popen_to_string("ip -d -s -j link show");
	printf("%s\n", json);

	jsmn_parser p;
	jsmn_init(&p);
	jsmntok_t t[TOK_COUNT];
	int r = jsmn_parse(&p, json, strlen(json), t, TOK_COUNT);
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		n = -1;
		goto error;
	}

	

	/* Assume the top-level element is an object */
	if (t[0].type != JSMN_ARRAY) {
		printf("Array expected\n");
		n = -1;
		goto error;
	}

	jsmntok_t * u = t + 1;
	n = ECS_MIN(t[0].size, out_length);
	for(int i = 0; i < n; ++i) {
		u = parse1(json, u, out + i);
	}

error:
	ecs_os_free(json);
	return n;
}