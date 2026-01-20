#pragma once
#include <flecs.h>

#define EG_KEYBOARDS_KEYS_MAX 512

#define EG_KEYBOARDS_STATE_RELEASED 0x01
#define EG_KEYBOARDS_STATE_PRESSED 0x02
#define EG_KEYBOARDS_STATE_DOWN 0x04
#define EG_KEYBOARDS_STATE_UP 0x08

typedef struct
{
	int32_t id;
} EgKeyboardsDevice;

typedef struct
{
	uint8_t state[EG_KEYBOARDS_KEYS_MAX];
} EgKeyboardsState;

// KeyActionToggleEntity

typedef struct
{
	int32_t key_index;
	ecs_entity_t entity;
	ecs_entity_t toggle;
} EgKeyboardsActionToggleEntity;

extern ECS_COMPONENT_DECLARE(EgKeyboardsDevice);
extern ECS_COMPONENT_DECLARE(EgKeyboardsState);
extern ECS_COMPONENT_DECLARE(EgKeyboardsActionToggleEntity);

void EgKeyboardsImport(ecs_world_t *world);


