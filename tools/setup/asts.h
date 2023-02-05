#pragma once
#include <flecs.h>


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

typedef enum
{
PARSE_MACHINE_ROOT,
PARSE_MACHINE_ASSIGNMENT,
PARSE_MACHINE_FUNCTION_DECLARTION_ARGUMENTS,
PARSE_MACHINE_EXPRESSION,
PARSE_MACHINE_STATEMENTS,
PARSE_MACHINE_FORLOOP,
PARSE_MACHINE_COUNT
} parse_machine_t;





#define AST_STACK_COUNT 128
typedef struct
{
	ecs_world_t * world;
	char const * text_start;
	char const * text_current;

	int32_t sp;
	ast_state_t ssta[AST_STACK_COUNT];
	ast_token_t stok[AST_STACK_COUNT];
	ecs_entity_t sent[AST_STACK_COUNT];
	int32_t spre[AST_STACK_COUNT];

	int32_t genid;
} ast_context_t;



void ast_parse(ast_context_t * ast);