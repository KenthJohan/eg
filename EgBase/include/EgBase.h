#pragma once
#include <flecs.h>


typedef struct
{
	char *value;
} EgBaseText;

typedef struct
{
	uint32_t color;
} EgBaseColor;

typedef struct
{
	float font_size;
	uint32_t color;
} EgBaseFont;


extern ECS_COMPONENT_DECLARE(EgBaseText);
extern ECS_COMPONENT_DECLARE(EgBaseFont);
extern ECS_COMPONENT_DECLARE(EgBaseColor);

void EgBaseImport(ecs_world_t *world);
