#pragma once
#include <flecs.h>
#include "tokens.h"
#include "lexer.h"

typedef enum
{
AST_STATE_UNKNOWN,
AST_STATE_ROOT,
AST_STATE_BRANCHES,
	AST_STATE_IF,
	AST_STATE_ELSEIF,
	AST_STATE_ELSE,
AST_STATE_EXPRESSION,
AST_STATE_BLOCK,
AST_STATE_STATEMENT,
AST_STATE_FORLOOP_ARGS,
AST_STATE_ASSIGNMENT,
AST_STATE_FUNCTION_DECLARTION_ARGUMENTS,
AST_STATE_COUNT
} ast_state_t;






#define AST_STACK_COUNT 128
typedef struct
{
	ecs_world_t * world;
	lexer_t lexer;

	int32_t sp;
	ast_state_t stack_state[AST_STACK_COUNT];
	ast_token_t stack_token[AST_STACK_COUNT];
	ecs_entity_t stack_entity[AST_STACK_COUNT];
	int32_t stack_precedence[AST_STACK_COUNT];

	int32_t genid;
} ast_context_t;



void ast_parse(ast_context_t * ast);