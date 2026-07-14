#pragma once
#include <flecs.h>

#define EG_BUTTONS_KEYS_MAX 512
#define EG_BUTTONS_MOUSEBUTTONS_MAX 512

#define EG_BUTTONS_STATE_HELD     0x01
#define EG_BUTTONS_STATE_PRESSED  0x02
#define EG_BUTTONS_STATE_RELEASED 0x04

typedef struct
{
	int32_t id;
} EgButtonsDevice;

typedef struct
{
	uint8_t state[EG_BUTTONS_KEYS_MAX];
	uint8_t mouse[EG_BUTTONS_MOUSEBUTTONS_MAX];
} EgButtonsState;

typedef struct
{
	int32_t key0;
	int32_t key1;
	uint8_t mask;
	ecs_entity_t entity;
	ecs_id_t comonent;
	uint8_t byte_offset;
	float factor;
} EgButtonsBinding;

typedef struct
{
	int32_t key;
	uint8_t mask;
	ecs_entity_t entity;
	ecs_entity_t relation;
	ecs_entity_t toggle;
} EgButtonsActionToggleEntity;

extern ECS_COMPONENT_DECLARE(EgButtonsDevice);
extern ECS_COMPONENT_DECLARE(EgButtonsState);
extern ECS_COMPONENT_DECLARE(EgButtonsBinding);
extern ECS_COMPONENT_DECLARE(EgButtonsActionToggleEntity);

void EgButtonsImport(ecs_world_t *world);
