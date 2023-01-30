#pragma once
#include <flecs.h>
//#include "../../vendor/flecs.h"

#define EG_NODE_TYPE_NUMBER 1
#define EG_NODE_TYPE_ADD 2
#define EG_NODE_TYPE_MUL 3

typedef struct eg_astnode_t eg_astnode_t;
struct eg_astnode_t
{
	int type;
	union
	{
		struct AST_NUMBER { int number; } value;
		struct AST_ADD { eg_astnode_t *left; eg_astnode_t *right; } add;
		struct AST_MUL { eg_astnode_t *left; eg_astnode_t *right; } mul;
	} data;
};


void eg_astnode_parse(ecs_world_t * world, char const * text);