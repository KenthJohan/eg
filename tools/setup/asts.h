#pragma once
#include <flecs.h>
#include "token.h"
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


typedef enum
{
PARSE_STATE_UNKNOWN,
PARSE_STATE_ROOT,
PARSE_STATE_STATEMENT,
PARSE_STATE_EXPR,
PARSE_STATE_EXPO,
PARSE_STATE_COUNT
} parse_state_t;

typedef enum
{
ACTION_UNKNOWN,
ACTION_NOP,
ACTION_PUSH,
ACTION_POP,
ACTION_PUSH_ADD_CHILD,
ACTION_PUSH_ADD_PARENT_PRECEDENCE,
} action_t;

typedef enum
{
AST_ERROR_NONE,
AST_ERROR_STACK_OVERFLOW,
AST_ERROR_STACK_UNDERFLOW
} ast_error_t;


#define AST_STACK_COUNT 128
typedef struct
{
	ecs_world_t * world;
	lexer_t lexer;

	int32_t sp;
	parse_state_t stack_state[AST_STACK_COUNT];
	tok_t         stack_tok[AST_STACK_COUNT];
	ecs_entity_t  stack_entity[AST_STACK_COUNT];
	int32_t       stack_precedence[AST_STACK_COUNT];

	int32_t genid;
} ast_context_t;




char const * ast_error_t_tostr(ast_error_t e);
ast_error_t ast_parse(ast_context_t * ast);