// popen() + read() causes crash without _DEFAULT_SOURCE
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "iplink.h"
#include "json.h"
#include <stdio.h>
#include <flecs.h>
#include <stdlib.h>

//
static char const *popen_to_string(char const *command)
{
	FILE *fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failed to run command\n");
		return NULL;
	}

	char buf[1024];
	ecs_strbuf_t b = ECS_STRBUF_INIT;
	while (1) {
		int n = fread(buf, 1, sizeof(buf), fp);
		if (n <= 0) {
			break;
		}
		ecs_strbuf_appendstrn(&b, buf, n);
	}
	pclose(fp);
	return ecs_strbuf_get(&b);
}

#define TOK_COUNT 1024

static jsmntok_t *parse1(char const *json, jsmntok_t *u, iplink_info_t *out)
{
	if (u->type != JSMN_OBJECT) {
		printf("Array expected\n");
	}
	u++;
	json_parse_value(json, u, u->parent, (char const *[]){"ifindex", NULL}, &out->ifindex, JSON_TYPE_INT);
	json_parse_string(json, u, u->parent, (char const *[]){"ifname", NULL}, out->ifname, 128);
	json_parse_string(json, u, u->parent, (char const *[]){"link_type", NULL}, out->link_type, 128);
	json_parse_value(json, u, u->parent, (char const *[]){"mtu", NULL}, &out->mtu, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"linkinfo", "info_data", "clock", NULL}, &out->can_clock, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"linkinfo", "info_data", "bittiming", "bitrate", NULL}, &out->can_bitrate, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"stats64", "rx", "bytes", NULL}, &out->stats64_rx_bytes, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"stats64", "rx", "packets", NULL}, &out->stats64_rx_packets, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"stats64", "rx", "errors", NULL}, &out->stats64_rx_errors, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"stats64", "tx", "bytes", NULL}, &out->stats64_tx_bytes, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"stats64", "tx", "packets", NULL}, &out->stats64_tx_packets, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"stats64", "tx", "errors", NULL}, &out->stats64_tx_errors, JSON_TYPE_INT);

	json_parse_value(json, u, u->parent, (char const *[]){"num_tx_queues", NULL}, &out->num_tx_queues, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"num_rx_queues", NULL}, &out->num_rx_queues, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"tso_max_size", NULL}, &out->tso_max_size, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"min_mtu", NULL}, &out->minmtu, JSON_TYPE_INT);
	json_parse_value(json, u, u->parent, (char const *[]){"max_mtu", NULL}, &out->maxmtu, JSON_TYPE_INT);


	

	while (1) {
		if (u->parent == 0) {
			break;
		}
		u++;
	}
	return u;
}


/*
https://docs.kernel.org/networking/statistics.html
https://man7.org/linux/man-pages/man8/ip.8.html
*/
int iplink_parse(iplink_info_t out[], int out_length)
{
	int n = 0;
	char const *json = popen_to_string("ip -d -s -j link show");
	if(json == NULL) {
		return -1;
	}
	//printf("%s\n", json);

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

	jsmntok_t *u = t + 1;
	n = ECS_MIN(t[0].size, out_length);
	for (int i = 0; i < n; ++i) {
		u = parse1(json, u, out + i);
	}

error:
	ecs_os_free((void*)json);
	return n;
}