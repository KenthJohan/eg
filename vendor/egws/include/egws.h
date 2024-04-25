#pragma once
#include <flecs.h>




typedef struct
{
	ecs_world_t * world;
	ecs_http_server_t *srv;
	ecs_entity_t root;
	int rc;
} egws_context_t;


typedef struct
{
	egws_context_t * impl;
	ecs_entity_t root;
} EgwsContext;


extern ECS_COMPONENT_DECLARE(EgwsContext);



void EgWebsocketsImport(ecs_world_t *world);