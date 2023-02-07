#include "asts.h"
#include "str.h"
#include "tokens.h"
#include <stdio.h>

#define BUFLEN 256

char const * ast_get_token_color(ast_token_t token)
{
	switch (token)
	{
	case AST_TOKEN_NUMBER: return "#FFF0FF";
	case AST_TOKEN_ID: return "#FFF0FF";
	case AST_TOKEN_MUL: return "#FF0000";
	case AST_TOKEN_PLUS: return "#F0F000";
	default: return "#FFFFFF";
	}
}


void ast_push(ast_context_t * ast, char const * name, ast_state_t state, ast_token_t token, int32_t precedence)
{


/*
	int32_t sp;
	for(sp = ast->sp-1; sp > 0; --sp)
	{
		if(precedence < ast->stack_precedence[sp])
		{
			break;
		}
	}
	*/

	{
		ecs_entity_t g = ecs_get_scope(ast->world);
		if(g){printf("Adding %s to %s\n", name, ecs_doc_get_name(ast->world, g));}
	}


	ecs_entity_t e = ecs_new_entity(ast->world, 0);
    ecs_doc_set_color(ast->world, e, ast_get_token_color(token));
	ecs_doc_set_name(ast->world, e, name);
	ast->stack_entity[ast->sp] = ecs_set_scope(ast->world, e);


	ast->sp++;
	ast->stack_entity[ast->sp] = e;
	ast->stack_state[ast->sp] = state;
	ast->stack_token[ast->sp] = token;
	ast->stack_precedence[ast->sp] = precedence;
}

void ast_pop(ast_context_t * ast)
{
	ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
	ast->stack_state[ast->sp] = 0;
	ast->stack_entity[ast->sp] = 0;
	ast->stack_token[ast->sp] = 0;
	ast->stack_precedence[ast->sp] = 0;
	ast->sp--;
}





void setup_ast_entity(ecs_world_t * world, ecs_entity_t e, ast_token_t token, char const * name)
{
	char buf[BUFLEN];
	char const * t = ast_get_tokenstr(token);
	snprintf(buf, BUFLEN, "%s%s", t, name);
	ecs_doc_set_color(world, e, ast_get_token_color(token));
	ecs_doc_set_name(world, e, buf);
}









void child_swap(ecs_entity_t e1, ecs_entity_t e2)
{

}






void ast_parse(ast_context_t * ast)
{
	{
		ecs_entity_t root = ecs_new_entity(ast->world, 0);
		ecs_doc_set_name(ast->world, root, "ASTROOT");
		ecs_set_scope(ast->world, root);
		ast->stack_entity[ast->sp] = root;
		ast->stack_state[ast->sp] = AST_STATE_ROOT;
		ast->stack_token[ast->sp] = AST_TOKEN_UNKNOWN;
		ast->stack_precedence[ast->sp] = 0;
	}



machine_goto:
	switch (ast->stack_state[ast->sp])
	{
	case AST_STATE_ROOT:
	case AST_STATE_IF:
	case AST_STATE_BLOCK:
		goto machine_codeblock;
	case AST_STATE_STATEMENT:
		goto machine_statement;
	case AST_STATE_EXPRESSION:
		goto machine_expression;
	}




machine_statement:
	while(1)
	{
		char buf[BUFLEN];
		ast_token_t token = tokens_next(&ast->text_current, buf, BUFLEN);
		switch (token)
		{
		case AST_TOKEN_EOF: return;

		case AST_TOKEN_ID:{
			ecs_entity_t e = ecs_new_entity(ast->world, 0);
			ecs_doc_set_name(ast->world, e, buf);
			break;}

		case AST_TOKEN_EQUAL:
			ast_push(ast, "EXPRESSION", AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_STATEMENT_TERMINATOR:
			ast_pop(ast);
			goto machine_goto;

		} // End of switch
	} // End of while





machine_codeblock:
	while(1)
	{
		char buf[BUFLEN];
		ast_token_t token = tokens_next(&ast->text_current, buf, BUFLEN);
		switch (token)
		{
		case AST_TOKEN_EOF: return;

		case AST_TOKEN_BLOCK_OPEN:
			ast_push(ast, "BLOCK", AST_STATE_BLOCK, token, 0);
			goto machine_codeblock;

		case AST_TOKEN_EQUAL:
			ast_push(ast, "EXPRESSION", AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_EXP_OPEN:
			ast_push(ast, "EXPRESSION", AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_BLOCK_CLOSE:
			ast_pop(ast);
			char const * p0 = ast->text_current;
			ast_token_t tokenpeek = tokens_next(&p0, NULL, 0);
			switch (tokenpeek)
			{
			case AST_TOKEN_ELSE:
			case AST_TOKEN_ELSEIF:
				ast_pop(ast);
				goto machine_codeblock;
			}
			goto machine_goto;


		case AST_TOKEN_IF:
			ast_push(ast, "IFCASE", AST_STATE_IFCASE, AST_TOKEN_UNKNOWN, 0);
			ast_push(ast, "IF", AST_STATE_IF, token, 0);
			goto machine_codeblock;

		case AST_TOKEN_ELSE:
			ast_push(ast, "ELSE", AST_STATE_ELSE, token, 0);
			goto machine_codeblock;

		case AST_TOKEN_ID:
			ast_push(ast, "STATEMENT", AST_STATE_STATEMENT, token, 0);
			goto machine_statement;


		} // End of switch
	} // End of while






machine_expression:
	while(1)
	{
		char buf[BUFLEN];
		ast_token_t token = tokens_next(&ast->text_current, buf, BUFLEN);
		switch (token)
		{
		case AST_TOKEN_EOF:
			return;

		case AST_TOKEN_DIV:
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), token, "");
			break;

		case AST_TOKEN_MUL:
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), token, "");
			break;

		case AST_TOKEN_PLUS:
			if(tokens_precedence[token]){}
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), token, "");
			break;

		case AST_TOKEN_ID:
			{
				ast_push(ast, "PLACEHOLDER", AST_STATE_EXPRESSION, AST_TOKEN_UNKNOWN, 0);
				ecs_entity_t e = ecs_new_entity(ast->world, 0);
			    ecs_doc_set_color(ast->world, e, ast_get_token_color(token));
				ecs_doc_set_name(ast->world, e, buf);
			}
			break;

		
		case AST_TOKEN_EXP_OPEN:
			ast_push(ast, "PLACEHOLDER", AST_STATE_EXPRESSION, token, 0);
			break;

		case AST_TOKEN_EXP_CLOSE:
			while(1)
			{
				if(ast->stack_state[ast->sp] == AST_TOKEN_EXP_OPEN)
				{
					ast_pop(ast);
					break;
				}
				if(ast->stack_state[ast->sp] != AST_STATE_EXPRESSION)
				{
					ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
					printf("set_scope %s\n", ecs_doc_get_name(ast->world, ecs_get_scope(ast->world)));
					break;
				}
				ast->stack_entity[ast->sp] = 0;
				ast->stack_token[ast->sp] = 0;
				ast->stack_precedence[ast->sp] = 0;
				ast->stack_state[ast->sp] = 0;
				ast->sp--;
			}
			goto machine_goto;

		case AST_TOKEN_STATEMENT_TERMINATOR:
			while(1)
			{
				if(ast->stack_state[ast->sp] != AST_STATE_EXPRESSION){break;}
				ast->stack_entity[ast->sp] = 0;
				ast->stack_token[ast->sp] = 0;
				ast->stack_precedence[ast->sp] = 0;
				ast->stack_state[ast->sp] = 0;
				ast->sp--;
			}
			ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
			goto machine_goto;



		} // End of switch
	} // End of while





}