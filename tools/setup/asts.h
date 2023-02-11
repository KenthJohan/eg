#pragma once
#include <flecs.h>
#include "token.h"
#include "lexer.h"

typedef enum
{
AST_PARSE_UNKNOWN,
AST_PARSE_ROOT,
AST_PARSE_STATEMENT,
AST_PARSE_EXPR,
AST_PARSE_EXPO,
AST_PARSE_COUNT
} ast_parse_t;

typedef enum
{
ACTION_UNKNOWN,
ACTION_NOP,
ACTION_PUSH,
ACTION_POP,
ACTION_PUSH_ADD_CHILD,
ACTION_INSERT_PARENT_PRECEDENCE,
} action_t;

typedef enum
{
AST_ERROR_NONE,
AST_ERROR_STACK_OVERFLOW,
AST_ERROR_STACK_UNDERFLOW,
AST_ERROR_OUT_OF_RANGE,
AST_ARROR_COUNT
} ast_error_t;


#define AST_STACK_COUNT 128
typedef struct
{
	ecs_world_t * world;
	lexer_t lexer;

	int32_t sp;
	ast_parse_t stack_parse[AST_STACK_COUNT];
	tok_t         stack_tok[AST_STACK_COUNT];
	ecs_entity_t  stack_entity[AST_STACK_COUNT];
	int32_t       stack_precedence[AST_STACK_COUNT];

	int32_t genid;
} ast_context_t;




char const * ast_error_t_tostr(ast_error_t e);
ast_error_t ast_parse(ast_context_t * ast);