#pragma once
#include <flecs.h>
#include "token.h"
#include "asts.h"

typedef struct
{
	ecs_i32_t value;
} EgAstCode;

extern ECS_TAG_DECLARE(EgAstEroot);
extern ECS_TAG_DECLARE(EgAstIf);
extern ECS_TAG_DECLARE(EgAstLeft);
extern ECS_TAG_DECLARE(EgAstRight);
extern ECS_TAG_DECLARE(EgAstCondition);
extern ECS_TAG_DECLARE(EgAstContent);
extern ECS_COMPONENT_DECLARE(EgAstCode);

void EgAstImport(ecs_world_t *world);

ecs_entity_t EgAst_newent(ecs_world_t * world, token_t * token, ast_parse_t parse);