#pragma once

#include "flecs.h"

#ifdef __cplusplus
extern "C" {
#endif


extern ECS_DECLARE(EgLogsError);
extern ECS_DECLARE(EgLogsVerbose);
extern ECS_DECLARE(EgLogsInfo);




#define EG_TRACE(world, channel, ...) eg_trace((world), (channel), __FILE__, __LINE__, __VA_ARGS__)
void eg_trace(ecs_world_t * world, ecs_entity_t channel, const char *file, int32_t line, const char *fmt, ...);







void EgLogsImport(ecs_world_t *world);




#ifdef __cplusplus
}
#endif
