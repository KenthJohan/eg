#pragma once

// Fixes: error: variable ‘ifr’ has initializer but incomplete type
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <flecs.h>
#include <stdint.h>

typedef struct {
	char const * interface;
} CanBusDescription;

typedef struct {
	ecs_i32_t socket;
} CanBus;

/*
https://docs.openvehicles.com/en/latest/components/vehicle_dbc/docs/dbc-primer.html
https://www.csselectronics.com/pages/can-dbc-file-database-intro
https://canlogger.csselectronics.com/dbc-editor/v129/dbc-editor.html
*/

typedef struct {
	uint32_t canid;
	uint8_t value;
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
} CanSignal;

extern ECS_COMPONENT_DECLARE(CanBus);
extern ECS_COMPONENT_DECLARE(CanSignal);

void CanImport(ecs_world_t *world);
