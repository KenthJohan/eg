#pragma once

#include "flecs.h"

#ifdef __cplusplus
extern "C" {
#endif


extern ECS_DECLARE(EgLogsVerbose);
extern ECS_DECLARE(EgLogsInfo);
extern ECS_DECLARE(EgLogsWarning);
extern ECS_DECLARE(EgLogsError);

extern ECS_DECLARE(EgEventsIdling);
extern ECS_DECLARE(EgEventsPrint);


#define EG_LOGS_COLOR_ERROR "#FF1111"
#define EG_LOGS_COLOR_VERBOSE "#47B5FF"
#define EG_LOGS_COLOR_INFO "#CCCCFF"
#define EG_LOGS_COLOR_WARNING "#FFFF76"


#define EG_EVENT_STRF(world, channel, ...) eg_event_strf((world), (channel), __FILE__, __LINE__, __VA_ARGS__)
void eg_event_strf(ecs_world_t * world, ecs_entity_t channel, const char *file, int32_t line, const char *fmt, ...);







void EgLogsImport(ecs_world_t *world);




#ifdef __cplusplus
}
#endif
