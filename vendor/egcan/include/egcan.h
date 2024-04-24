#pragma once

// Fixes: error: variable ‘ifr’ has initializer but incomplete type
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <flecs.h>
#include <stdint.h>

typedef struct {
	void * internal;
} EgCanEpoll;

typedef struct {
	char const * interface;
	int32_t signals_amount;
} EgCanBusDescription;

typedef struct {
	ecs_i32_t socket;
} EgCanBus;



#define EG_CAN_MAX_DLC 8
#define EG_CAN_MAX_RAW_DLC 15
#define EG_CAN_MAX_DLEN 8
#define EG_CAN_MAX_ID 2031 // 0x7EF
typedef struct {
	uint32_t can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
	uint8_t len;      // CAN frame payload length in byte (0 .. EG_CAN_MAX_DLEN)
	uint8_t __pad;    // padding
	uint8_t __res0;   // reserve
	uint8_t len8_dlc; // optional DLC for 8 byte payload length (9 .. 15)
	uint8_t data[EG_CAN_MAX_DLEN] __attribute__((aligned(8)));
} eg_can_frame_t;


#define EG_CAN_BOOK_CAP 1024
typedef struct {
	int len;
	int dirty;
	uint8_t packet[8];
} eg_can_book8_t;

typedef struct {
	int32_t socket;
	int32_t cap;
	eg_can_book8_t * tx;
	eg_can_book8_t * rx;
} EgCanBusBook;

/*
https://docs.openvehicles.com/en/latest/components/vehicle_dbc/docs/dbc-primer.html
https://www.csselectronics.com/pages/can-dbc-file-database-intro
https://canlogger.csselectronics.com/dbc-editor/v129/dbc-editor.html
*/

typedef struct {
	uint32_t canid;
	int32_t len;
	int32_t rx;
	int32_t tx;
	int32_t byte_offset;
	int32_t min;
	int32_t max;
	int32_t gui_index;
	/*
	uint8_t type;
	uint8_t order;
	uint8_t mode;
	uint8_t start;
	uint8_t length;
	float factor;
	uint8_t offset;
	uint8_t min;
	*/
} EgCanSignal;

extern ECS_COMPONENT_DECLARE(EgCanEpoll);
extern ECS_COMPONENT_DECLARE(EgCanBusDescription);
extern ECS_COMPONENT_DECLARE(EgCanBus);
extern ECS_COMPONENT_DECLARE(EgCanSignal);
extern ECS_COMPONENT_DECLARE(EgCanBusBook);

void EgCanBusBook_prepare_send(EgCanBusBook * book, EgCanSignal * signal);

void EgCanImport(ecs_world_t *world);
