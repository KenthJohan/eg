#pragma once



#include <flecs.h>
#include <stdint.h>
#include <egquantities.h>

typedef struct {
	int32_t dummy;
	void * impl;
} EgCanRxThread;

typedef struct {
	int32_t dummy;
} EgCanRxThreadMember;

typedef struct {
	char const * interface;
	int32_t signals_amount;
	int32_t error;
} EgCanBusDescription;


#define EG_CAN_BOOK_CAP 1024
typedef struct {
	uint32_t len;
	uint32_t can_id;
	uint32_t dirty;
	uint32_t stats_count;
	uint8_t payload[8];
	ecs_time_t time;
} eg_can_book_packet8_t;

typedef struct {
	int32_t sock;
	uint32_t cap;
	eg_can_book_packet8_t * tx;
	eg_can_book_packet8_t * rx;
} eg_can_book_t;

typedef struct {
	int32_t socket;
	eg_can_book_t * ptr;
} EgCanBus;







/*
https://docs.openvehicles.com/en/latest/components/vehicle_dbc/docs/dbc-primer.html
https://www.csselectronics.com/pages/can-dbc-file-database-intro
https://canlogger.csselectronics.com/dbc-editor/v129/dbc-editor.html
*/

typedef struct {
	uint8_t len;
	uint8_t byte_offset;
	uint8_t rxtx;
} EgCanSignal;

typedef struct {
	uint32_t id;
	uint32_t n;
	double elapsed;
	uint32_t flags;
} EgCanId;

extern ECS_DECLARE(EgCanRx);
extern ECS_DECLARE(EgCanTx);
extern ECS_COMPONENT_DECLARE(EgCanRxThread);
extern ECS_COMPONENT_DECLARE(EgCanRxThreadMember);
extern ECS_COMPONENT_DECLARE(EgCanBusDescription);
extern ECS_COMPONENT_DECLARE(EgCanBus);
extern ECS_COMPONENT_DECLARE(EgCanSignal);
extern ECS_COMPONENT_DECLARE(EgCanId);

void EgCan_book_prepare_send(eg_can_book_t * book, EgCanSignal * signal, EgCanId * channel, EgQuantitiesRangedGeneric * value);

void EgCanImport(ecs_world_t *world);








