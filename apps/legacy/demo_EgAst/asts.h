#pragma once
#include <flecs.h>
#include "token.h"
#include "lexer.h"


#define AST_TITLE_FORMAT "%-10s : "

typedef enum
{
AST_NODE_UNKNOWN,
AST_NODE_STATEMENT,
AST_NODE_ASSIGNMENT,
AST_NODE_EXP_ID,
AST_NODE_EXP_PLUS,
AST_NODE_EXP_MINUS,
AST_NODE_EXP_MUL,
AST_NODE_EXP_DIV,
AST_NODE_COUNT
} ast_node_t;

typedef enum
{
AST_PARSE_UNKNOWN,
AST_PARSE_UNKNOWN_STACK,
AST_PARSE_ROOT,
AST_PARSE_STATEMENT,
AST_PARSE_EROOT,
AST_PARSE_EXPR,
AST_PARSE_EXPO,
AST_PARSE_COUNT
} ast_parse_t;

typedef enum
{
AST_ACTION_UNKNOWN,
AST_ACTION_NOP,
AST_ACTION_POP1,
AST_ACTION_POP2,
AST_ACTION_POP_ANTISCOPE,
AST_ACTION_ADD,
AST_ACTION_PUSH_ADD_CHILD,
AST_ACTION_INSERT_PARENT_PRECEDENCE,
} ast_action_t;

typedef enum
{
AST_ERROR_NONE,
AST_ERROR_UNKNOWN,
AST_ERROR_NULL,
AST_ERROR_UNHANDLED_STATE,
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
	ast_parse_t   stack_parse      [AST_STACK_COUNT];
	tok_t         stack_tok        [AST_STACK_COUNT];
	ecs_entity_t  stack_entity     [AST_STACK_COUNT];
	int32_t       stack_precedence [AST_STACK_COUNT];

	int32_t genid;

	ecs_entity_t (*newent)(ecs_world_t * world, token_t * token, ast_parse_t parse);

} ast_context_t;




char const * ast_error_t_tostr(ast_error_t e);
ast_error_t ast_parse(ast_context_t * ast);