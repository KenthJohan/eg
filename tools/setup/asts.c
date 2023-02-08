#include "asts.h"
#include "str.h"
#include "tokens.h"
#include <stdio.h>

#define BUFLEN 256

char const * ast_get_state_color(ast_state_t state)
{
	switch (state)
	{
	case AST_STATE_ROOT: return "#444444";
	case AST_STATE_BRANCHES: return "#880088";
	case AST_STATE_IF: return "#880088";
	case AST_STATE_ELSEIF: return "#880088";
	case AST_STATE_ELSE: return "#880088";
	case AST_STATE_STATEMENT: return "#F0F00F";
	case AST_STATE_BLOCK: return "#00F0FF";
	case AST_STATE_EXPRESSION: return "#0F0FFF";
	default: return "#FFFFFF";
	}
}

char const * ast_state_t_str(ast_state_t state)
{
	switch (state)
	{
	case AST_STATE_UNKNOWN: return "UNKNOWN";
	case AST_STATE_ROOT: return "ROOT";
	case AST_STATE_BRANCHES: return "BRANCHES";
	case AST_STATE_IF: return "IF";
	case AST_STATE_ELSEIF: return "ELSEIF";
	case AST_STATE_ELSE: return "ELSE";
	case AST_STATE_EXPRESSION: return "EXPRESSION";
	case AST_STATE_BLOCK: return "BLOCK";
	case AST_STATE_STATEMENT: return "STATEMENT";
	case AST_STATE_FORLOOP_ARGS: return "FORLOOP_ARGS";
	case AST_STATE_ASSIGNMENT: return "ASSIGNMENT";
	case AST_STATE_FUNCTION_DECLARTION_ARGUMENTS: return "FUNCTION_DECLARTION_ARGUMENTS";
	case AST_STATE_COUNT: return "AST_STATE_COUNT";
	default: return "";
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
	if(name == NULL)
	{
		name = ast_state_t_str(state);
	}
	
	{
		ecs_entity_t g = ecs_get_scope(ast->world);
		if(g){printf("Adding %s to %s\n", name, ecs_doc_get_name(ast->world, g));}
	}


	ecs_entity_t e = ecs_new_entity(ast->world, 0);
    ecs_doc_set_color(ast->world, e, ast_get_state_color(state));

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
	ast->stack_state[ast->sp] = 0;
	ast->stack_entity[ast->sp] = 0;
	ast->stack_token[ast->sp] = 0;
	ast->stack_precedence[ast->sp] = 0;
	ast->sp--;
	ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
}





void setup_ast_entity(ecs_world_t * world, ecs_entity_t e, ast_state_t state, ast_token_t token, char const * name)
{
	char buf[BUFLEN];
	char const * t = ast_get_tokenstr(token);
	snprintf(buf, BUFLEN, "%s%s", t, name);
	ecs_doc_set_color(world, e, ast_get_state_color(state));
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
	printf("machine_goto\n");
	switch (ast->stack_state[ast->sp])
	{
	case AST_STATE_ROOT:
	case AST_STATE_BLOCK:
		goto machine_codeblock;
	case AST_STATE_ELSEIF:
	case AST_STATE_ELSE:
	case AST_STATE_IF:
		goto machine_ifcase;
	case AST_STATE_STATEMENT:
		goto machine_statement;
	case AST_STATE_EXPRESSION:
		goto machine_expression;
	default:
		printf("machine_goto: state %s not handled\n", ast_state_t_str(ast->stack_state[ast->sp]));
		goto machine_error;
	}


machine_statement:
	printf("machine_statement\n");
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
			ast_push(ast, NULL, AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_STATEMENT_TERMINATOR:
			ast_pop(ast);
			goto machine_goto;

		} // End of switch
	} // End of while


machine_ifcase:
	printf("machine_ifcase\n");
	while(1)
	{
		char buf[BUFLEN];
		ast_token_t token = tokens_next(&ast->text_current, buf, BUFLEN);
		switch (token)
		{
		case AST_TOKEN_EOF: return;

		case AST_TOKEN_EXP_OPEN:
			ast_push(ast, NULL, AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_ELSE:
			ast_pop(ast);
			ast_push(ast, NULL, AST_STATE_ELSE, token, 0);
			goto machine_ifcase;

		case AST_TOKEN_ELSEIF:
			ast_pop(ast);
			ast_push(ast, NULL, AST_TOKEN_ELSEIF, token, 0);
			goto machine_ifcase;

		case AST_TOKEN_BLOCK_OPEN:
			ast_push(ast, NULL, AST_STATE_BLOCK, token, 0);
			goto machine_codeblock;

		default:
			ast_pop(ast);
			goto machine_goto;

		} // End of switch
	} // End of while


machine_codeblock:
	printf("machine_codeblock\n");
	while(1)
	{
		char buf[BUFLEN];
		ast_token_t token = tokens_next(&ast->text_current, buf, BUFLEN);
		switch (token)
		{
		case AST_TOKEN_EOF: return;

		case AST_TOKEN_BLOCK_OPEN:
			ast_push(ast, NULL, AST_STATE_BLOCK, token, 0);
			goto machine_codeblock;

		case AST_TOKEN_EQUAL:
			ast_push(ast, NULL, AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_EXP_OPEN:
			ast_push(ast, NULL, AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_BLOCK_CLOSE:
			ast_pop(ast);
			goto machine_goto;

		case AST_TOKEN_IF:
			ast_push(ast, NULL, AST_STATE_BRANCHES, AST_TOKEN_UNKNOWN, 0);
			ast_push(ast, NULL, AST_STATE_IF, token, 0);
			goto machine_ifcase;

		case AST_TOKEN_ELSE:
			ast_push(ast, NULL, AST_STATE_ELSE, token, 0);
			goto machine_codeblock;

		case AST_TOKEN_ID:
			ast_push(ast, NULL, AST_STATE_STATEMENT, token, 0);
			goto machine_statement;


		} // End of switch
	} // End of while


machine_expression:
	printf("machine_expression\n");
	while(1)
	{
		char buf[BUFLEN];
		ast_token_t token = tokens_next(&ast->text_current, buf, BUFLEN);
		switch (token)
		{
		case AST_TOKEN_EOF:
			return;

		case AST_TOKEN_DIV:
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), AST_STATE_EXPRESSION, token, "");
			break;

		case AST_TOKEN_MUL:
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), AST_STATE_EXPRESSION, token, "");
			break;

		case AST_TOKEN_PLUS:
			if(tokens_precedence[token]){}
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), AST_STATE_EXPRESSION, token, "");
			break;

		case AST_TOKEN_ID:
			{
				ast_push(ast, "PLACEHOLDER", AST_STATE_EXPRESSION, AST_TOKEN_UNKNOWN, 0);
				ecs_entity_t e = ecs_new_entity(ast->world, 0);
			    ecs_doc_set_color(ast->world, e, ast_get_state_color(AST_STATE_EXPRESSION));
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


machine_error:
	printf("machine_error: AST machine error\n");
	return;

machine_eof:
	printf("machine_eof: AST machine end of file\n");
	return;

}