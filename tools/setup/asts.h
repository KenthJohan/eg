#pragma once
#include <flecs.h>



/*
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
*/


typedef enum
{
AST_STATE_UNKNOWN,
AST_STATE_ROOT,
AST_STATE_IFCASE,
	AST_STATE_IF,
	AST_STATE_ELSEIF,
	AST_STATE_ELSE,
AST_STATE_EXPRESSION,
AST_STATE_BLOCK,
AST_STATE_STATEMENT,
AST_STATE_COUNT
} ast_state_t;





#define AST_STACK_COUNT 128
typedef struct
{
	char const * text_start;
	char const * text_current;

	int32_t sp;
	ast_state_t stack[AST_STACK_COUNT];
	ecs_entity_t stack1[AST_STACK_COUNT];

	ecs_entity_t estack[AST_STACK_COUNT];
	int32_t genid;
} ast_context_t;



void ast_parse(ecs_world_t * world, ast_context_t * ast);