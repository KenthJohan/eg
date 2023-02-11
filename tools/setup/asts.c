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










char const * parse_state_t_tostr(parse_state_t state)
{
	switch (state)
	{
	case PARSE_STATE_UNKNOWN: return "PARSE_STATE_UNKNOWN";
	case PARSE_STATE_ROOT: return "PARSE_STATE_ROOT";
	case PARSE_STATE_STATEMENT: return "PARSE_STATE_STATEMENT";
	case PARSE_STATE_EXPR: return "PARSE_STATE_EXPR";
	case PARSE_STATE_EXPO: return "PARSE_STATE_EXPO";
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
	case ACTION_PUSH_ADD_CHILD: return "ACTION_PUSH_ADD_CHILD";
	case ACTION_PUSH_ADD_PARENT_PRECEDENCE: return "ACTION_PUSH_ADD_PARENT_PRECEDENCE";
	default: return "?";
	}
}


char const * ast_error_t_tostr(ast_error_t e)
{
	switch (e)
	{
	case AST_ERROR_NONE: return "AST_ERROR_NONE";
	case AST_ERROR_STACK_OVERFLOW: return "AST_ERROR_STACK_OVERFLOW";
	case AST_ERROR_STACK_UNDERFLOW: return "AST_ERROR_STACK_UNDERFLO";
	default: return "?";
	}
}

typedef struct 
{
	parse_state_t state;
	action_t action;
} state_t;

state_t table_root[]=
{
	[TOK_ID       ] = {PARSE_STATE_STATEMENT, ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT    ] = {0}
};
state_t table_statement[]=
{
	[TOK_EQUAL    ] = {PARSE_STATE_EXPR, ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT    ] = {0}
};
state_t table_expo[]=
{
	[TOK_PLUS     ] = {PARSE_STATE_EXPR, ACTION_PUSH_ADD_PARENT_PRECEDENCE},
	[TOK_MINUS    ] = {PARSE_STATE_EXPR, ACTION_PUSH_ADD_PARENT_PRECEDENCE},
	[TOK_MUL      ] = {PARSE_STATE_EXPR, ACTION_PUSH_ADD_PARENT_PRECEDENCE},
	[TOK_SEMICOLON] = {PARSE_STATE_EXPR, ACTION_PUSH_ADD_PARENT_PRECEDENCE},
	[TOK_COUNT    ] = {0}
};
state_t table_expr[]=
{
	[TOK_ID     ] = {PARSE_STATE_EXPO, ACTION_PUSH_ADD_CHILD},
	[TOK_NUMBER ] = {PARSE_STATE_EXPO, ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT  ] = {0}
};
state_t * tables[] = 
{
	[PARSE_STATE_ROOT] = table_root,
	[PARSE_STATE_STATEMENT] = table_statement,
	[PARSE_STATE_EXPR] = table_expr,
	[PARSE_STATE_EXPO] = table_expo,
	[PARSE_STATE_COUNT] = 0
};





#define BUFLEN 128
ecs_entity_t newent(ecs_world_t * world, token_t * token)
{
	ecs_entity_t s = ecs_get_scope(world);
	char buf[BUFLEN];
	int32_t l = token->length;
	ecs_entity_t e;
	switch (token->tok)
	{
	case TOK_ID:
		l = l < BUFLEN ? l : BUFLEN-1;
		memcpy(buf, token->cursor, l);
		buf[l] = '\0';
		e = ecs_new_entity(world, 0);
		ecs_doc_set_name(world, e, buf);
		break;
	
	default:
		e = ecs_new_entity(world, 0);
		ecs_doc_set_name(world, e, tok_t_tostr(token->tok));
		break;
	}
	printf("New %s: %s\n", ecs_doc_get_name(world, s), ecs_doc_get_name(world, e));
	return e;
}



ast_error_t ast_parse(ast_context_t * ast)
{
	ecs_entity_t root = ecs_new_entity(ast->world, 0);
	ecs_doc_set_name(ast->world, root, "ASTROOT");
	ecs_set_scope(ast->world, root);
	ast->stack_entity[ast->sp] = root;
	ast->stack_state[ast->sp] = AST_STATE_ROOT;
	ast->stack_tok[ast->sp] = TOK_UNKNOWN;
	ast->stack_precedence[ast->sp] = 0;

	parse_state_t current = PARSE_STATE_ROOT;
	action_t action = ACTION_NOP;
	ast->sp = 0;
	ast->stack_tok[ast->sp] = TOK_UNKNOWN;
	ast->stack_state[ast->sp] = current;
	ast->stack_entity[ast->sp] = root;
	ast->stack_precedence[ast->sp] = 0;


	token_t token;
	ecs_entity_t e;
	while(1)
	{
		printf("State: %s\n", parse_state_t_tostr(current), action_t_tostr(action));

		lexer_next(&ast->lexer, &token);
		printf("Token: %s: %s\n", tok_t_tostr(token.tok), token.cursor);
		if(token.tok == TOK_EOF) {break;}

		action = tables[current][token.tok].action;
		printf("Action: %s\n", action_t_tostr(action));

		switch (action)
		{
		case ACTION_PUSH_ADD_CHILD:
			e = newent(ast->world, &token);
			ecs_set_scope(ast->world, e);
			ast->sp++;
			if(ast->sp >= AST_STACK_COUNT){return AST_ERROR_STACK_OVERFLOW;}
			ast->stack_tok[ast->sp] = token.tok;
			ast->stack_state[ast->sp] = current;
			ast->stack_entity[ast->sp] = e;
			ast->stack_precedence[ast->sp] = 0;
			break;

		case ACTION_PUSH_ADD_PARENT_PRECEDENCE:{
				int32_t p1 = tok_t_precedence[token.tok];
				int32_t p2 = ast->stack_precedence[ast->sp-1];
				if(p1 < p2)
				{
					ast->stack_tok[ast->sp] = 0;
					ast->stack_state[ast->sp] = 0;
					ast->stack_entity[ast->sp] = 0;
					ast->stack_precedence[ast->sp] = 0;
					ast->sp--;
					if(ast->sp < 0){return AST_ERROR_STACK_UNDERFLOW;}
				}
				ecs_set_scope(ast->world, ast->stack_entity[ast->sp-1]);
				e = newent(ast->world, &token);
				printf("Add pair %s EcsChildOf %s\n", ecs_doc_get_name(ast->world, ast->stack_entity[ast->sp]), ecs_doc_get_name(ast->world, e));
				ecs_add_pair(ast->world, ast->stack_entity[ast->sp], EcsChildOf, e);
				ecs_set_scope(ast->world, e);
				ast->stack_tok[ast->sp] = token.tok;
				ast->stack_state[ast->sp] = current;
				ast->stack_entity[ast->sp] = e;
				ast->stack_precedence[ast->sp] = tok_t_precedence[token.tok];
			}break;
		
		default:
			break;
		}
		current = tables[current][token.tok].state;
	}


	return AST_ERROR_NONE;
}