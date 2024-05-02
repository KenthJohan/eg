// #define _GNU_SOURCE /* To get defns of NI_MAXSERV and NI_MAXHOST */

// Fixes: error: variable ‘ifr’ has initializer but incomplete type
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "egcan.h"
#include <egquantities.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/epoll.h>
#include <errno.h>
#include <assert.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

#include "egifaces.h"
#include "iplink.h"

ECS_COMPONENT_DECLARE(EgIfacesDetails);

ECS_CTOR(EgIfacesDetails, ptr, {
	ecs_os_memset_t(ptr, 0, EgIfacesDetails);
})

ECS_DTOR(EgIfacesDetails, ptr, {
	ecs_os_free(ptr->link_type);
})

ECS_MOVE(EgIfacesDetails, dst, src, {
	ecs_os_free(dst->link_type);
	*dst = *src;
	// This makes sure the value doesn't get deleted twice,
	// as the destructor is still invoked after a move:
	src->link_type = NULL;
})

// The copy hook should copy resources from one location to another.
ECS_COPY(EgIfacesDetails, dst, src, {
	ecs_trace("Copy");
	ecs_os_free(dst->link_type);
	*dst = *src;
	dst->link_type = ecs_os_strdup(src->link_type);
})

void Tick(ecs_iter_t *it)
{

	// FILE * fp = popen("ip -d -s -j link show", "r");
	// test_popen(fp);

	iplink_info_t info[5] = {0};
	int n = 0;
	n = iplink_parse(info, 5);

	for (int i = 0; i < n; ++i) {
		char buf[256];
		snprintf(buf, sizeof(buf), "interfaces.%s", info[i].ifname);
		ecs_entity_t a = ecs_new_entity(it->world, buf);
		EgIfacesDetails ptr = {
		.link_type = info[i].link_type,
		.can_bitrate = info[i].can_bitrate,
		.can_clock = info[i].can_clock,
		.index = info[i].ifindex,
		.mtu = info[i].mtu,
		.tso_max_size = info[i].tso_max_size,
		.numtxqueues = info[i].num_tx_queues,
		.numrxqueues = info[i].num_rx_queues,
		.minmtu = info[i].minmtu,
		.maxmtu = info[i].maxmtu,
		.stats64_rx_bytes = info[i].stats64_rx_bytes,
		.stats64_rx_packets = info[i].stats64_rx_packets,
		.stats64_rx_errors = info[i].stats64_rx_errors,
		.stats64_tx_bytes = info[i].stats64_tx_bytes,
		.stats64_tx_packets = info[i].stats64_tx_packets,
		.stats64_tx_errors = info[i].stats64_tx_errors,
		};
		ecs_set_ptr(it->world, a, EgIfacesDetails, &ptr);
	}

	return;
}

void EgIfacesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgIfaces);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "EgIfaces");

	ECS_COMPONENT_DEFINE(world, EgIfacesDetails);

	// clang-format off
	ecs_struct(world,
	{.entity = ecs_id(EgIfacesDetails),
	.members = {
	{.name = "index", .type = ecs_id(ecs_i32_t)},
	{.name = "link_type", .type = ecs_id(ecs_string_t)},
	{.name = "bitrate", .type = ecs_id(ecs_i32_t)},
	{.name = "clock", .type = ecs_id(ecs_i32_t)},
	{.name = "tso_max_size", .type = ecs_id(ecs_i32_t)},
	{.name = "numtxqueues", .type = ecs_id(ecs_i32_t)},
	{.name = "numrxqueues", .type = ecs_id(ecs_i32_t)},
	{.name = "minmtu", .type = ecs_id(ecs_i32_t)},
	{.name = "maxmtu", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_rx_bytes", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_rx_packets", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_rx_errors", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_tx_bytes", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_tx_packets", .type = ecs_id(ecs_i32_t)},
	{.name = "stats64_tx_errors", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_system(world, {
	.entity = ecs_entity(world, { 
	.name = "Tick",
	.add = { ecs_dependson(EcsOnUpdate) } // run in OnUpdate phase
	}),
	.callback = Tick,
	.interval = 1.0  // time in seconds
	});

	// clang-format on
}