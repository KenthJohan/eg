#pragma once

#include "flecs.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef struct
{
	char * value;
} EgName;


typedef struct
{
	uint32_t value;
} EgVersion;

typedef uint32_t EgIndex;


extern ECS_COMPONENT_DECLARE(EgName);
extern ECS_COMPONENT_DECLARE(EgVersion);
extern ECS_COMPONENT_DECLARE(EgVkIndex);
extern ECS_COMPONENT_DECLARE(EgIndex);

extern ECS_DECLARE(EgApplication);
extern ECS_DECLARE(EgEngine);
extern ECS_DECLARE(EgApi);

void EgTypesImport(ecs_world_t *world);


#ifdef __cplusplus
}
#endif
