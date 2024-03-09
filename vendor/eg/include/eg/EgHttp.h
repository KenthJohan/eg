#pragma once

#include <flecs.h>



typedef struct
{
	ecs_http_server_t *srv;
	ecs_entity_t root;
} EgHttp;


extern ECS_COMPONENT_DECLARE(EgHttp);

void EgHttpServersImport(ecs_world_t *world);
