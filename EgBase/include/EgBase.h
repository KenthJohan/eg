#pragma once
#include <flecs.h>


typedef struct
{
	char *value;
} EgBaseText;

typedef struct
{
	float font_size;
} EgBaseFont;

typedef struct
{
	uint32_t color;
} EgBaseColor;


extern ECS_COMPONENT_DECLARE(EgBaseText);
extern ECS_COMPONENT_DECLARE(EgBaseFont);
extern ECS_COMPONENT_DECLARE(EgBaseColor);

void EgBaseImport(ecs_world_t *world);
