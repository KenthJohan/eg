#pragma once
#include <flecs.h>

typedef struct
{
	uint16_t port; /**< Port of server (optional, default = 7681) */
	char *ipaddr;  /**< Interface address (optional, default = 0.0.0.0) */
	void *impl;
} EglwsContext;

extern ECS_COMPONENT_DECLARE(EglwsContext);

void WebsocketsImport(ecs_world_t *world);