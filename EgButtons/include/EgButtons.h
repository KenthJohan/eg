#pragma once
#include <flecs.h>

#define EG_BUTTONS_SCANCODES_MAX 512
#define EG_BUTTONS_MOUSE_MAX     16

#define EG_BUTTONS_STATE_HELD     0x01 // Button is currently held down
#define EG_BUTTONS_STATE_PRESSED  0x02 // Button was pressed this frame
#define EG_BUTTONS_STATE_RELEASED 0x04 // Button was released this frame

typedef struct
{
	int32_t id;
} EgButtonsDevice;

typedef struct
{
	uint8_t scancode[EG_BUTTONS_SCANCODES_MAX];
	uint8_t mouse[EG_BUTTONS_MOUSE_MAX];
} EgButtonsState;

uint8_t EgButtonsState_get(EgButtonsState *state, uint32_t key);

typedef struct
{
	int32_t button0;
	int32_t button1;
	uint8_t mask;
} EgButtonsBinding;

typedef struct
{
	ecs_entity_t entity;
	ecs_id_t     component;
	uint8_t      byte_offset;
	float        factor;
	float        w0;
	float        w1;
} EgButtonsIncrementer;

typedef struct
{
	ecs_entity_t entity;
	ecs_entity_t relation;
	ecs_entity_t toggle;
} EgButtonsActionToggleEntity;

typedef struct
{
	ecs_id_t     term;
	ecs_entity_t tag;
	bool         add;
} EgButtonsEngageRule;

extern ECS_COMPONENT_DECLARE(EgButtonsDevice);
extern ECS_COMPONENT_DECLARE(EgButtonsState);
extern ECS_COMPONENT_DECLARE(EgButtonsBinding);
extern ECS_COMPONENT_DECLARE(EgButtonsIncrementer);
extern ECS_COMPONENT_DECLARE(EgButtonsActionToggleEntity);
extern ECS_COMPONENT_DECLARE(EgButtonsEngageRule);

void EgButtonsImport(ecs_world_t *world);
