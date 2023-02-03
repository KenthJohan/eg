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


typedef enum
{
AST_TOKEN_UNKNOWN,
AST_TOKEN_EOF,
AST_TOKEN_WHITESPACE,
AST_TOKEN_BLOCK_OPEN,
AST_TOKEN_BLOCK_CLOSE,
AST_TOKEN_EXP_OPEN,
AST_TOKEN_EXP_CLOSE,
AST_TOKEN_STATEMENT_TERMINATOR,
AST_TOKEN_IF,
AST_TOKEN_NUMBER,
AST_TOKEN_COUNT
} ast_token_t;


#define AST_STACK_COUNT 128
typedef struct
{
	int32_t sp;
	ast_state_t stack[AST_STACK_COUNT];
	ecs_entity_t stack1[AST_STACK_COUNT];
} ast_context_t;



void ast_parse(ecs_world_t * world, ast_context_t * ast, char const * text);