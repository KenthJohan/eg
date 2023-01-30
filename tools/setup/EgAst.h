#pragma once
#include <flecs.h>


typedef struct
{
	ecs_i32_t value;
} EgAstCode;



extern ECS_TAG_DECLARE(EgAstIf);
extern ECS_TAG_DECLARE(EgAstLeft);
extern ECS_TAG_DECLARE(EgAstRight);
extern ECS_TAG_DECLARE(EgAstCondition);
extern ECS_TAG_DECLARE(EgAstContent);
extern ECS_COMPONENT_DECLARE(EgAstCode);



void EgAstImport(ecs_world_t *world);


void eg_ast_parse(ecs_world_t *world, char const * text);