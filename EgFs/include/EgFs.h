#pragma once
#include <flecs.h>

typedef struct
{
	ecs_vec_t buf;
} EgFsContent;

typedef struct
{
	void * stream;
} EgFsStream;


extern ECS_COMPONENT_DECLARE(EgFsContent);
extern ECS_COMPONENT_DECLARE(EgFsStream);

extern ECS_TAG_DECLARE(EgFsDir);
extern ECS_TAG_DECLARE(EgFsFile);
extern ECS_TAG_DECLARE(EgFsUpdate);
extern ECS_TAG_DECLARE(EgFsRead);
extern ECS_TAG_DECLARE(EgFsPrint);
extern ECS_TAG_DECLARE(EgFsEof);

void EgFsImport(ecs_world_t *world);
