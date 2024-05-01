#pragma once



#include <flecs.h>
#include <stdint.h>



typedef struct {
	int32_t index;
	int32_t can_bitrate;
	int32_t can_clock;
	int32_t mtu;
	int32_t tso_max_size;
	int32_t numtxqueues;
	int32_t numrxqueues;
	int32_t minmtu;
	int32_t maxmtu;
	int32_t stats64_rx_bytes;
	int32_t stats64_rx_packets;
	int32_t stats64_rx_errors;
	int32_t stats64_tx_bytes;
	int32_t stats64_tx_packets;
	int32_t stats64_tx_errors;
} EgIfacesDetails;


extern ECS_COMPONENT_DECLARE(EgIfacesDetails);

void EgIfacesImport(ecs_world_t *world);
