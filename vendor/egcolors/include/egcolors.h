#pragma once
#include <flecs.h>


typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} EgColorsV4U8_RGBA;

typedef struct {
	float r;
	float g;
	float b;
	float a;
} EgColorsV4F32_RGBA;

extern ECS_COMPONENT_DECLARE(EgColorsV4U8_RGBA);
extern ECS_COMPONENT_DECLARE(EgColorsV4F32_RGBA);

void EgColorsImport(ecs_world_t *world);