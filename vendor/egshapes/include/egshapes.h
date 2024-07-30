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
	float width;
	float height;
	float depth;
	int32_t random_colors;
} Box;

typedef struct {
	float a[3];
	float b[3];
} Line;

typedef struct {
	float w;
	float h;
} Rectangle;


extern ECS_COMPONENT_DECLARE(Box);
extern ECS_COMPONENT_DECLARE(Torus);
extern ECS_COMPONENT_DECLARE(Cylinder);
extern ECS_COMPONENT_DECLARE(Sphere);
extern ECS_COMPONENT_DECLARE(Line);
extern ECS_COMPONENT_DECLARE(Rectangle);

void EgShapesImport(ecs_world_t *world);