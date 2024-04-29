#pragma once
#include <flecs.h>


typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} EgColorsRGBA8888;


extern ECS_COMPONENT_DECLARE(EgColorsRGBA8888);

void EgColorsImport(ecs_world_t *world);