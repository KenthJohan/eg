#pragma once

#include <flecs.h>


typedef struct {
	double k0;
	double k1;
} EgPolynomialsLinear;


extern ECS_COMPONENT_DECLARE(EgPolynomialsLinear);

void EgPolynomialsImport(ecs_world_t *world);