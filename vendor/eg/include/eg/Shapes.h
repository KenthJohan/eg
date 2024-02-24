#pragma once

#include <flecs.h>


typedef struct {
	float radius;
	float ring_radius;
	int32_t rings;
	int32_t sides;
	int32_t random_colors;
} Torus;

typedef struct {
	float radius;
	float height;
	int32_t slices;
	int32_t stacks;
	int32_t random_colors;
} Cylinder;

typedef struct {
	float radius;
	int32_t slices;
	int32_t stacks;
	int32_t random_colors;
} Sphere;

typedef struct {
	float a[3];
	float b[3];
} Line;


extern ECS_COMPONENT_DECLARE(Torus);
extern ECS_COMPONENT_DECLARE(Cylinder);
extern ECS_COMPONENT_DECLARE(Sphere);
extern ECS_COMPONENT_DECLARE(Line);

void ShapesImport(ecs_world_t *world);