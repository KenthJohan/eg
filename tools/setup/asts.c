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










char const * ast_parse_t_tostr(ast_parse_t state)
{
	switch (state)
	{
	case AST_PARSE_UNKNOWN: return "UNKNOWN";
	case AST_PARSE_ROOT: return "ROOT";
	case AST_PARSE_STATEMENT: return "STATEMENT";
	case AST_PARSE_EXPR: return "EXPR";
	case AST_PARSE_EXPO: return "EXPO";
	case AST_PARSE_COUNT: return "COUNT";
	default: return "?";
	}
}

char const * action_t_tostr(action_t state)
{
	switch (state)
	{
	case ACTION_UNKNOWN: return "UNKNOWN";
	case ACTION_NOP: return "NOP";
	case ACTION_PUSH: return "PUSH";
	case ACTION_POP: return "POP";
	case ACTION_PUSH_ADD_CHILD: return "PUSH_ADD_CHILD";
	case ACTION_INSERT_PARENT_PRECEDENCE: return "INSERT_PARENT_PRECEDENCE";
	default: return "?";
	}
}


char const * ast_error_t_tostr(ast_error_t e)
{
	switch (e)
	{
	case AST_ERROR_NONE: return "AST_ERROR_NONE";
	case AST_ERROR_STACK_OVERFLOW: return "AST_ERROR_STACK_OVERFLOW";
	case AST_ERROR_STACK_UNDERFLOW: return "AST_ERROR_STACK_UNDERFLOW";
	case AST_ERROR_OUT_OF_RANGE: return "AST_ERROR_OUT_OF_RANGE";
	case AST_ARROR_COUNT: return "AST_ARROR_COUNT";
	default: return "?";
	}
}

typedef struct 
{
	ast_parse_t state;
	action_t action;
} state_t;

state_t table_root[]=
{
	[TOK_ID       ] = {AST_PARSE_STATEMENT, ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT    ] = {0}
};
state_t table_statement[]=
{
	[TOK_EQUAL    ] = {AST_PARSE_EXPR, ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT    ] = {0}
};
state_t table_expo[]=
{
	[TOK_PLUS     ] = {AST_PARSE_EXPR, ACTION_INSERT_PARENT_PRECEDENCE},
	[TOK_MINUS    ] = {AST_PARSE_EXPR, ACTION_INSERT_PARENT_PRECEDENCE},
	[TOK_MUL      ] = {AST_PARSE_EXPR, ACTION_INSERT_PARENT_PRECEDENCE},
	[TOK_SEMICOLON] = {AST_PARSE_EXPR, ACTION_INSERT_PARENT_PRECEDENCE},
	[TOK_COUNT    ] = {0}
};
state_t table_expr[]=
{
	[TOK_ID     ] = {AST_PARSE_EXPO, ACTION_PUSH_ADD_CHILD},
	[TOK_NUMBER ] = {AST_PARSE_EXPO, ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT  ] = {0}
};
state_t * tables[] = 
{
	[AST_PARSE_ROOT] = table_root,
	[AST_PARSE_STATEMENT] = table_statement,
	[AST_PARSE_EXPR] = table_expr,
	[AST_PARSE_EXPO] = table_expo,
	[AST_PARSE_COUNT] = 0
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
	printf("[%-10s] "ECS_GREY"%s"ECS_NORMAL" > "ECS_BLUE"%s"ECS_NORMAL"\n", "New",
		ecs_doc_get_name(world, s), 
		ecs_doc_get_name(world, e));
	return e;
}



ast_error_t ast_parse(ast_context_t * ast)
{
	ecs_entity_t root = ecs_new_entity(ast->world, 0);
	ecs_doc_set_name(ast->world, root, "ASTROOT");
	ecs_set_scope(ast->world, root);
	ast->stack_entity    [ast->sp] = root;
	ast->stack_parse     [ast->sp] = AST_PARSE_ROOT;
	ast->stack_tok       [ast->sp] = TOK_UNKNOWN;
	ast->stack_precedence[ast->sp] = 0;

	ast_parse_t current = AST_PARSE_ROOT;
	ast->sp = 0;
	ast->stack_tok        [ast->sp] = TOK_UNKNOWN;
	ast->stack_parse      [ast->sp] = current;
	ast->stack_entity     [ast->sp] = root;
	ast->stack_precedence [ast->sp] = 0;


	while(1)
	{
		token_t token;
		lexer_next(&ast->lexer, &token);
		if(token.tok == TOK_EOF) {break;}
		action_t action = tables[current][token.tok].action;

		
		printf("[%-10s] "ECS_YELLOW"%-10s"ECS_NORMAL" "ECS_MAGENTA"%-10s"ECS_NORMAL" "ECS_GREEN"%-10s"ECS_NORMAL"\n",
			"State", 
			ast_parse_t_tostr(current),
			      tok_t_tostr(token.tok),
			   action_t_tostr(action)
		);


		switch (action)
		{
		case ACTION_PUSH_ADD_CHILD:{
			ecs_entity_t e = newent(ast->world, &token);
			ecs_set_scope(ast->world, e);
			ast->sp++;
			if(ast->sp >= AST_STACK_COUNT){return AST_ERROR_STACK_OVERFLOW;}
			ast->stack_tok        [ast->sp] = token.tok;
			ast->stack_parse      [ast->sp] = current;
			ast->stack_entity     [ast->sp] = e;
			ast->stack_precedence [ast->sp] = 0;
			break;}

		case ACTION_INSERT_PARENT_PRECEDENCE:{
				int32_t p1 = tok_t_precedence[token.tok];
				int32_t p2 = ast->stack_precedence[ast->sp-1];
				if(p1 < p2)
				{
					ast->stack_tok        [ast->sp] = 0;
					ast->stack_parse      [ast->sp] = 0;
					ast->stack_entity     [ast->sp] = 0;
					ast->stack_precedence [ast->sp] = 0;
					ast->sp--;
					if(ast->sp < 0){return AST_ERROR_STACK_UNDERFLOW;}
				}
				ecs_set_scope(ast->world, ast->stack_entity[ast->sp-1]);
				ecs_entity_t e = newent(ast->world, &token);
				printf("[%-10s] "ECS_BLUE"%s"ECS_NORMAL" EcsChildOf "ECS_BLUE"%s"ECS_NORMAL"\n",
					"Pair",
					ecs_doc_get_name(ast->world, ast->stack_entity[ast->sp]),
					ecs_doc_get_name(ast->world, e));
				ecs_add_pair(ast->world, ast->stack_entity[ast->sp], EcsChildOf, e);
				ecs_set_scope(ast->world, e);
				ast->stack_tok        [ast->sp] = token.tok;
				ast->stack_parse      [ast->sp] = current;
				ast->stack_entity     [ast->sp] = e;
				ast->stack_precedence [ast->sp] = tok_t_precedence[token.tok];
			}break;
		
		default:
			break;
		}
		current = tables[current][token.tok].state;
	}


	return AST_ERROR_NONE;
}