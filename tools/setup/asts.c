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




char const * parse_state_t_tostr(parse_state_t state)
{
	switch (state)
	{
	case PARSE_STATE_UNKNOWN: return "PARSE_STATE_UNKNOWN";
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



typedef struct 
{
	parse_state_t state;
	action_t action;
} state_t;





state_t table_expo[]=
{
	[TOK_PLUS] = {PARSE_STATE_EXPR, ACTION_PUSH_ADD_PARENT_PRECEDENCE},
	[TOK_MUL] = {PARSE_STATE_EXPR, ACTION_PUSH_ADD_PARENT_PRECEDENCE},
	[TOK_SEMICOLON] = {PARSE_STATE_EXPR, ACTION_PUSH_ADD_PARENT_PRECEDENCE},
	[TOK_COUNT] = {0}
};

state_t table_expr[]=
{
	[TOK_ID] = {PARSE_STATE_EXPO, ACTION_PUSH_ADD_CHILD},
	[TOK_NUMBER] = {PARSE_STATE_EXPO, ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT] = {0}
};


state_t * tables[] = 
{
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



#define STACK_SIZE 128

void ast_parse(ast_context_t * ast)
{
	
	ecs_entity_t root = ecs_new_entity(ast->world, 0);
	ecs_doc_set_name(ast->world, root, "ASTROOT");
	ecs_set_scope(ast->world, root);
	ast->stack_entity[ast->sp] = root;
	ast->stack_state[ast->sp] = AST_STATE_ROOT;
	ast->stack_token[ast->sp] = TOK_UNKNOWN;
	ast->stack_precedence[ast->sp] = 0;
	



	parse_state_t current = PARSE_STATE_EXPR;
	action_t action = ACTION_NOP;

	tok_t stack_tok[STACK_SIZE] = {0};
	parse_state_t stack_parstate[STACK_SIZE] = {0};
	ecs_entity_t stack_ents[STACK_SIZE] = {0};

	int32_t sp = 0;
	stack_tok[sp] = TOK_UNKNOWN;
	stack_parstate[sp] = current;
	stack_ents[sp] = root;

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
			sp++;
			stack_tok[sp] = token.tok;
			stack_parstate[sp] = current;
			stack_ents[sp] = e;
			break;

		case ACTION_PUSH_ADD_PARENT_PRECEDENCE:{
				//printf("Remove pair %s EcsChildOf %s\n", ecs_doc_get_name(ast->world, stack_ents[sp]), ecs_doc_get_name(ast->world, stack_ents[sp-1]));

				int32_t p1 = tok_t_precedence[token.tok];
				int32_t p2 = tok_t_precedence[stack_tok[sp-1]];
				if(p2 && (p1 < p2))
				{
					stack_tok[sp] = 0;
					stack_parstate[sp] = 0;
					sp--;
				}
				
				ecs_set_scope(ast->world, stack_ents[sp-1]);
				e = newent(ast->world, &token);
				printf("Add pair %s EcsChildOf %s\n", ecs_doc_get_name(ast->world, stack_ents[sp]), ecs_doc_get_name(ast->world, e));
				ecs_add_pair(ast->world, stack_ents[sp], EcsChildOf, e);
				ecs_set_scope(ast->world, e);


				stack_tok[sp] = token.tok;
				stack_parstate[sp] = current;
				stack_ents[sp] = e;
			}break;
		
		default:
			break;
		}

		
		current = tables[current][token.tok].state;
	}

}