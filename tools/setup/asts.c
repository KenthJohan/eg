#include "asts.h"
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



typedef enum
{
	AST_NEWFLAGS_NONE = 0x00,
	AST_NEWFLAGS_PUSH = 0x01,
} ast_newflags_t;


void ast_new(ast_context_t * ast, token_t * token, ast_state_t state, int32_t precedence, int32_t flags)
{
	char name[BUFLEN] = {0};

	if(token)
	{
		if(token->tokconstant == TOKEN_CONSTANT_ID)
		{
			int32_t l = (token->length < BUFLEN) ? token->length : BUFLEN-1;
			memcpy(name, token->cursor, l);
			name[l] = '\0';
		}
		else
		{
			snprintf(name, BUFLEN, "%s", token_constant_t_tostr(token->tokconstant));
		}
	}
	else
	{
		snprintf(name, BUFLEN, "%s", ast_state_t_str(state));
	}

	ecs_assert(name[0] != '\0', ECS_INVALID_OPERATION, NULL);

	{
		ecs_entity_t g = ecs_get_scope(ast->world);
		if(g){printf("Adding %s %i to %s\n", name, token->length, ecs_doc_get_name(ast->world, g));}
		else{printf("Adding %s\n", name);}
	}

	ecs_entity_t e = ecs_new_entity(ast->world, 0);
    ecs_doc_set_color(ast->world, e, ast_get_state_color(state));
	ecs_doc_set_name(ast->world, e, name);

	if(flags & AST_NEWFLAGS_PUSH)
	{
		ast->stack_entity[ast->sp] = ecs_set_scope(ast->world, e);
		ast->sp++;
		ast->stack_entity[ast->sp] = e;
		ast->stack_state[ast->sp] = state;
		ast->stack_token[ast->sp] = token->tokconstant;
		ast->stack_precedence[ast->sp] = precedence;
	}
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





void setup_ast_entity(ecs_world_t * world, ecs_entity_t e, ast_state_t state, token_t * token, char const * name)
{
	char buf[BUFLEN];
	char const * t = token_constant_t_tostr(token->tokconstant);
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
		ast->stack_token[ast->sp] = TOKEN_CONSTANT_UNKNOWN;
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
		token_t token;
		lexer_next(&ast->lexer, &token);

		switch (token.tokconstant)
		{
		case TOKEN_CONSTANT_EOF: return;

		case TOKEN_CONSTANT_ID:
			ast_new(ast, &token, AST_STATE_EXPRESSION, 0, AST_NEWFLAGS_NONE);
			goto machine_statement;

		case TOKEN_CONSTANT_EQUAL:
			ast_new(ast, &token, AST_STATE_EXPRESSION, 0, AST_NEWFLAGS_PUSH);
			goto machine_expression;

		case TOKEN_CONSTANT_STATEMENT_TERMINATOR:
			ast_pop(ast);
			goto machine_goto;

		} // End of switch
	} // End of while


machine_ifcase:
	printf("machine_ifcase\n");
	while(1)
	{
		token_t token;
		lexer_next(&ast->lexer, &token);
		switch (token.tokconstant)
		{
		case TOKEN_CONSTANT_EOF: return;

		case TOKEN_CONSTANT_EXP_OPEN:
			ast_new(ast, &token, AST_STATE_EXPRESSION, 0, AST_NEWFLAGS_PUSH);
			goto machine_expression;

		case TOKEN_CONSTANT_ELSE:
			ast_pop(ast);
			ast_new(ast, &token, AST_STATE_ELSE, 0, AST_NEWFLAGS_PUSH);
			goto machine_ifcase;

		case TOKEN_CONSTANT_ELSEIF:
			ast_pop(ast);
			ast_new(ast, &token, AST_STATE_ELSEIF, 0, AST_NEWFLAGS_PUSH);
			goto machine_ifcase;

		case TOKEN_CONSTANT_BLOCK_OPEN:
			ast_new(ast, &token, AST_STATE_BLOCK, 0, AST_NEWFLAGS_PUSH);
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
		token_t token;
		lexer_next(&ast->lexer, &token);
		switch (token.tokconstant)
		{
		case TOKEN_CONSTANT_EOF: return;

		case TOKEN_CONSTANT_BLOCK_OPEN:
			ast_new(ast, &token, AST_STATE_BLOCK, 0, AST_NEWFLAGS_PUSH);
			goto machine_codeblock;

		case TOKEN_CONSTANT_EQUAL:
			ast_new(ast, &token, AST_STATE_EXPRESSION, 0, AST_NEWFLAGS_PUSH);
			goto machine_expression;

		case TOKEN_CONSTANT_EXP_OPEN:
			ast_new(ast, &token, AST_STATE_EXPRESSION, 0, AST_NEWFLAGS_PUSH);
			goto machine_expression;

		case TOKEN_CONSTANT_BLOCK_CLOSE:
			ast_pop(ast);
			goto machine_goto;

		case TOKEN_CONSTANT_IF:
			ast_new(ast, &token, AST_STATE_BRANCHES, 0, AST_NEWFLAGS_PUSH);
			ast_new(ast, &token, AST_STATE_IF, 0, AST_NEWFLAGS_PUSH);
			goto machine_ifcase;

		case TOKEN_CONSTANT_ELSE:
			ast_new(ast, &token, AST_STATE_ELSE, 0, AST_NEWFLAGS_PUSH);
			goto machine_codeblock;

		case TOKEN_CONSTANT_ID:
			ast_new(ast, &token, AST_STATE_STATEMENT, 0, AST_NEWFLAGS_PUSH);
			goto machine_statement;


		} // End of switch
	} // End of while


machine_expression:
	printf("machine_expression\n");
	while(1)
	{
		token_t token;
		lexer_next(&ast->lexer, &token);
		switch (token.tokconstant)
		{
		case TOKEN_CONSTANT_EOF:
			return;

		case TOKEN_CONSTANT_DIV:
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), AST_STATE_EXPRESSION, &token, "");
			break;

		case TOKEN_CONSTANT_MUL:
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), AST_STATE_EXPRESSION, &token, "");
			break;

		case TOKEN_CONSTANT_PLUS:
			setup_ast_entity(ast->world, ecs_get_scope(ast->world), AST_STATE_EXPRESSION, &token, "");
			break;

		case TOKEN_CONSTANT_ID:
			{
				ast_new(ast, &token, AST_STATE_EXPRESSION, TOKEN_CONSTANT_UNKNOWN, AST_NEWFLAGS_PUSH);
				ast_new(ast, &token, AST_STATE_EXPRESSION, TOKEN_CONSTANT_UNKNOWN, AST_NEWFLAGS_NONE);
			}
			break;

		
		case TOKEN_CONSTANT_EXP_OPEN:
			ast_new(ast, &token, AST_STATE_EXPRESSION, 0, AST_NEWFLAGS_PUSH);
			break;

		case TOKEN_CONSTANT_EXP_CLOSE:
			while(1)
			{
				if(ast->stack_state[ast->sp] == TOKEN_CONSTANT_EXP_OPEN)
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

		case TOKEN_CONSTANT_STATEMENT_TERMINATOR:
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