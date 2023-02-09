#include "asts.h"
#include <stdio.h>



/*
State    | Input       | Next       | Output
=========|=============|============|============================
ROOT     | SYMBOL      | DECL       | 
ROOT     | IF          | BRIF       | PUSH

BRIF     | PAREN_OPEN  | EXPL       | PUSH

BRANCHES | CURLY_OPEN  | EXPL       | PUSH
BRANCHES | ELSEIF      | BRANCHES   | PUSH
BRANCHES | ELSE        | BRANCHES   | PUSH

DECL     | SYMBOL      | DECL       | ADD_THIS
DECL     | ASSIGNMENT  | EXPL       | 
         
EXPL     | SYMBOL      | EXPO       | PUSH2
EXPO     | PLUS        | EXPR       | PUSH2
EXPO     | MUL         | EXPR       | PUSH2
EXPR     | SYMBOL      | ?          | 

EXP      | PAREN_OPEN  | EXP        | PUSH
EXP      | PAREN_CLOSE | ?          | POPX 
EXP      | TERMINATOR  | ?          | POPX





*/





typedef enum
{
PARSE_STATE_UNKNOWN,
PARSE_STATE_EXPL,
PARSE_STATE_EXPR,
PARSE_STATE_EXPO,
PARSE_STATE_POP,
PARSE_STATE_COUNT
} parse_state_t;

typedef enum
{
ACTION_UNKNOWN,
ACTION_NOP,
ACTION_PUSH,
ACTION_POP,
} action_t;




char const * parse_state_t_tostr(parse_state_t state)
{
	switch (state)
	{
	case PARSE_STATE_UNKNOWN: return "PARSE_STATE_UNKNOWN";
	case PARSE_STATE_EXPL: return "PARSE_STATE_EXPL";
	case PARSE_STATE_EXPR: return "PARSE_STATE_EXPR";
	case PARSE_STATE_EXPO: return "PARSE_STATE_EXPO";
	case PARSE_STATE_POP: return "PARSE_STATE_POP";
	case PARSE_STATE_COUNT: return "PARSE_STATE_COUNT";
	default: return "?";
	}
}

char const * action_t_tostr(action_t state)
{
	switch (state)
	{
	case ACTION_UNKNOWN: return "ACTION_UNKNOWN";
	case ACTION_NOP: return "ACTION_NOP";
	case ACTION_PUSH: return "ACTION_PUSH";
	case ACTION_POP: return "ACTION_POP";
	default: return "?";
	}
}



typedef struct 
{
	parse_state_t state;
	action_t action;
} state_t;




state_t table_expl[]=
{
	[TOK_ID] = {PARSE_STATE_EXPO, ACTION_NOP},
	[TOK_NUMBER] = {PARSE_STATE_EXPO, ACTION_NOP},
	[TOK_COUNT] = {0}
};

state_t table_expo[]=
{
	[TOK_PLUS] = {PARSE_STATE_EXPR, ACTION_NOP},
	[TOK_MUL] = {PARSE_STATE_EXPR, ACTION_NOP},
	[TOK_SEMICOLON] = {PARSE_STATE_EXPR, ACTION_NOP},
	[TOK_COUNT] = {0}
};

state_t table_expr[]=
{
	[TOK_ID] = {PARSE_STATE_EXPO, ACTION_NOP},
	[TOK_NUMBER] = {PARSE_STATE_EXPO, ACTION_POP},
	[TOK_COUNT] = {0}
};


void * tables[] = 
{
	[PARSE_STATE_EXPL] = table_expl,
	[PARSE_STATE_EXPR] = table_expr,
	[PARSE_STATE_EXPO] = table_expo,
	[PARSE_STATE_COUNT] = 0
};






void ast_parse(ast_context_t * ast)
{
	{
		ecs_entity_t root = ecs_new_entity(ast->world, 0);
		ecs_doc_set_name(ast->world, root, "ASTROOT");
		ecs_set_scope(ast->world, root);
		ast->stack_entity[ast->sp] = root;
		ast->stack_state[ast->sp] = AST_STATE_ROOT;
		ast->stack_token[ast->sp] = TOK_UNKNOWN;
		ast->stack_precedence[ast->sp] = 0;
	}



	parse_state_t stack[100];
	int32_t sp;
	token_t token;
	state_t next = {PARSE_STATE_EXPL, ACTION_NOP};
	while(1)
	{
		state_t * t = tables[next.state];
		printf("next %s %s\n", parse_state_t_tostr(next.state), action_t_tostr(next.action));
		lexer_next(&ast->lexer, &token);
		if(token.tok == TOK_EOF) {break;}
		next = t[token.tok];
	}

}