#pragma once



#include <flecs.h>
#include <stdint.h>
#include <egquantities.h>

typedef struct {
	uint32_t nodeid;
	uint32_t cmd;
} EgOdrivesCmd;


extern ECS_COMPONENT_DECLARE(EgOdrivesCmd);

void EgOdrivesImport(ecs_world_t *world);








