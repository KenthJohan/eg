#pragma once

#include <flecs.h>



typedef struct
{
	ecs_world_t * world;
	ecs_http_server_t *srv;
	ecs_entity_t root;
	int rc;
} eg_webserver_t;


typedef struct
{
	eg_webserver_t * impl;
	ecs_entity_t root;
} EgWebServer;


extern ECS_COMPONENT_DECLARE(EgWebServer);

void EgHttpServersImport(ecs_world_t *world);


ecs_entity_t EgHttp_add_file(ecs_world_t * world, ecs_entity_t rootnode, char const * filename);