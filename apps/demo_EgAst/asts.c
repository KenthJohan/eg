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


#define ASSERT(c) if (!(c)) __builtin_trap()






char const * ast_parse_t_tostr(ast_parse_t state)
{
	switch (state)
	{
	case AST_PARSE_UNKNOWN:         return "UNKNOWN";
	case AST_PARSE_UNKNOWN_STACK:   return "UNKNOWN_STACK";
	case AST_PARSE_ROOT:            return "ROOT";
	case AST_PARSE_STATEMENT:       return "STATEMENT";
	case AST_PARSE_EROOT:           return "EROOT";
	case AST_PARSE_EXPR:            return "EXPR";
	case AST_PARSE_EXPO:            return "EXPO";
	case AST_PARSE_COUNT:           return "COUNT";
	default:                        return "?";
	}
}

char const * action_t_tostr(ast_action_t state)
{
	switch (state)
	{
	case AST_ACTION_UNKNOWN:                  return "UNKNOWN";
	case AST_ACTION_NOP:                      return "NOP";
	case AST_ACTION_POP1:                     return "POP1";
	case AST_ACTION_POP2:                     return "POP2";
	case AST_ACTION_POP_ANTISCOPE:            return "POP_ANTISCOPE";
	case AST_ACTION_ADD:                      return "ADD";
	case AST_ACTION_PUSH_ADD_CHILD:           return "PUSH_ADD_CHILD";
	case AST_ACTION_INSERT_PARENT_PRECEDENCE: return "INSERT_PARENT_PRECEDENCE";
	default:                                  return "?";
	}
}


char const * ast_error_t_tostr(ast_error_t e)
{
	switch (e)
	{
	case AST_ERROR_NONE:            return "AST_ERROR_NONE";
	case AST_ERROR_UNKNOWN:         return "AST_ERROR_UNKNOWN";
	case AST_ERROR_NULL:            return "AST_ERROR_NULL";
	case AST_ERROR_UNHANDLED_STATE: return "AST_ERROR_UNHANDLED_STATE";
	case AST_ERROR_STACK_OVERFLOW:  return "AST_ERROR_STACK_OVERFLOW";
	case AST_ERROR_STACK_UNDERFLOW: return "AST_ERROR_STACK_UNDERFLOW";
	case AST_ERROR_OUT_OF_RANGE:    return "AST_ERROR_OUT_OF_RANGE";
	case AST_ARROR_COUNT:           return "AST_ARROR_COUNT";
	default:                        return "?";
	}
}



typedef struct 
{
	ast_parse_t state;
	ast_action_t action;
	union
	{
		ast_parse_t until_parse;
	};
} state_t;


// https://stackoverflow.com/questions/18820624/compilers-how-to-parse-function-calls-and-function-definitions

state_t table_root[]=
{
	[TOK_ID       ] = {AST_PARSE_STATEMENT, AST_ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT    ] = {0}
};
state_t table_statement[]=
{
	[TOK_ID       ] = {AST_PARSE_STATEMENT, AST_ACTION_ADD},
	[TOK_EQUAL    ] = {AST_PARSE_EROOT, AST_ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT    ] = {0}
};

state_t table_expo[]=
{
	[TOK_PLUS       ] = {AST_PARSE_EXPR,          AST_ACTION_INSERT_PARENT_PRECEDENCE},
	[TOK_MINUS      ] = {AST_PARSE_EXPR,          AST_ACTION_INSERT_PARENT_PRECEDENCE},
	[TOK_MUL        ] = {AST_PARSE_EXPR,          AST_ACTION_INSERT_PARENT_PRECEDENCE},
	//[TOK_COMMA      ] = {AST_PARSE_EXPR,          AST_ACTION_INSERT_PARENT_PRECEDENCE},
	[TOK_COMMA      ] = {AST_PARSE_EXPR,          AST_ACTION_POP1, .until_parse = AST_PARSE_EROOT},
	[TOK_SEMICOLON  ] = {AST_PARSE_UNKNOWN_STACK, AST_ACTION_POP2, .until_parse = AST_PARSE_STATEMENT},
	[TOK_PAREN_OPEN ] = {AST_PARSE_EROOT,         AST_ACTION_NOP},
	[TOK_PAREN_CLOSE] = {AST_PARSE_EXPO,          AST_ACTION_POP_ANTISCOPE},
	[TOK_COUNT      ] = {0}
};
state_t table_eroot[]=
{
	[TOK_ID         ] = {AST_PARSE_EXPO, AST_ACTION_PUSH_ADD_CHILD},
	[TOK_NUMBER     ] = {AST_PARSE_EXPO, AST_ACTION_PUSH_ADD_CHILD},
	[TOK_PAREN_OPEN ] = {AST_PARSE_EXPR, AST_ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT      ] = {0}
};
state_t table_expr[]=
{
	[TOK_ID         ] = {AST_PARSE_EXPO, AST_ACTION_PUSH_ADD_CHILD},
	[TOK_NUMBER     ] = {AST_PARSE_EXPO, AST_ACTION_PUSH_ADD_CHILD},
	[TOK_PAREN_OPEN ] = {AST_PARSE_EXPR, AST_ACTION_PUSH_ADD_CHILD},
	[TOK_COUNT      ] = {0}
};
state_t * tables[] = 
{
	[AST_PARSE_ROOT     ] = table_root,
	[AST_PARSE_STATEMENT] = table_statement,
	[AST_PARSE_EROOT    ] = table_eroot,
	[AST_PARSE_EXPR     ] = table_expr,
	[AST_PARSE_EXPO     ] = table_expo,
	[AST_PARSE_COUNT    ] = 0
};









void ast_pop(ast_context_t * ast)
{
	printf(AST_TITLE_FORMAT"\n", "[Pop]");
	ASSERT(ast);
	ast->stack_tok        [ast->sp] = 0;
	ast->stack_parse      [ast->sp] = 0;
	ast->stack_entity     [ast->sp] = 0;
	ast->stack_precedence [ast->sp] = 0;
	ast->sp--;
}



ast_error_t ast_parse(ast_context_t * ast)
{
	ASSERT(ast);
	ecs_entity_t root = ecs_new_entity(ast->world, 0);
	ecs_doc_set_name(ast->world, root, "ASTROOT");
	ecs_set_scope(ast->world, root);
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
		state_t * table = tables[current];

		
		printf(AST_TITLE_FORMAT ECS_YELLOW"%-10s"ECS_NORMAL" "ECS_MAGENTA"%-10s"ECS_NORMAL" "ECS_GREEN"%-10s"ECS_NORMAL"\n",
			"[State]", 
			ast_parse_t_tostr(current),
			      tok_t_tostr(token.tok),
			   action_t_tostr(table ? table[token.tok].action : AST_ACTION_UNKNOWN)
		);

		if(table == NULL) {return AST_ERROR_UNHANDLED_STATE;}
		table += token.tok;
		if(table->action == AST_ACTION_UNKNOWN) {return AST_ERROR_UNHANDLED_STATE;}

		
		current = tables[current][token.tok].state;

		switch (table->action)
		{
		case AST_ACTION_NOP:{
			break;}
		case AST_ACTION_ADD:{
			ecs_entity_t e = ast->newent(ast->world, &token, current);
			break;}
		case AST_ACTION_POP1:{ 
			while(1)
			{
				if(ast->sp <= 0){return AST_ERROR_STACK_UNDERFLOW;}
				if(ast->stack_parse[ast->sp] == table->until_parse)
				{
					//ast_pop(ast);
					break;
				}
				ast_pop(ast);
			}
			ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
			ecs_entity_t e = ast->newent(ast->world, &token, current);
			break;}
		case AST_ACTION_POP2:{ 
			while(1)
			{
				if(ast->sp <= 0){return AST_ERROR_STACK_UNDERFLOW;}
				if(ast->stack_parse[ast->sp] == table->until_parse)
				{
					ast_pop(ast);
					break;
				}
				ast_pop(ast);
			}
			ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
			ecs_entity_t e = ast->newent(ast->world, &token, current);
			break;}
		case AST_ACTION_POP_ANTISCOPE:
			while(1)
			{
				if(ast->stack_tok[ast->sp] == tok_t_antiscope[token.tok]){break;}
				if(ast->sp <= 0){return AST_ERROR_STACK_UNDERFLOW;}
				ast_pop(ast);
			}
			ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
			break;
			
		case AST_ACTION_PUSH_ADD_CHILD:{
			ecs_entity_t e = ast->newent(ast->world, &token, current);
			ecs_set_scope(ast->world, e);
			ast->sp++;
			if(ast->sp >= AST_STACK_COUNT){return AST_ERROR_STACK_OVERFLOW;}
			ast->stack_tok        [ast->sp] = token.tok;
			ast->stack_parse      [ast->sp] = current;
			ast->stack_entity     [ast->sp] = e;
			ast->stack_precedence [ast->sp] = 0;
			printf(AST_TITLE_FORMAT"\n", "[Push]");
			break;}

		case AST_ACTION_INSERT_PARENT_PRECEDENCE:{
				int32_t p1 = tok_t_precedence[token.tok];
				int32_t p2 = ast->stack_precedence[ast->sp-1];
				if(p1 < p2)
				{
					ast_pop(ast);
					if(ast->sp < 0){return AST_ERROR_STACK_UNDERFLOW;}
				}
				ecs_set_scope(ast->world, ast->stack_entity[ast->sp-1]);
				ecs_entity_t e = ast->newent(ast->world, &token, current);
				printf(AST_TITLE_FORMAT ECS_BLUE"%s"ECS_NORMAL" EcsChildOf "ECS_BLUE"%s"ECS_NORMAL"\n",
					"[Pair]",
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


		} // END switch

		if(current == AST_PARSE_UNKNOWN_STACK)
		{
			current = ast->stack_parse[ast->sp];
		}

	} // END while




	return AST_ERROR_NONE;
}