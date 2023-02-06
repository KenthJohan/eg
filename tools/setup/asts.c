#include "asts.h"
#include "str.h"
#include "tokens.h"
#include <stdio.h>

#define BUFLEN 128

void ast_push(ast_context_t * ast, char const * name, ast_state_t state, ast_token_t token, int32_t precedence)
{
	ast->genid++;
	char buf[BUFLEN];
	snprintf(buf, BUFLEN, "%s%i", name, ast->genid);

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


	ecs_entity_t e = ecs_new_entity(ast->world, buf);
	ast->stack_entity[ast->sp] = ecs_set_scope(ast->world, e);
	ast->stack_state[ast->sp] = state;
	ast->stack_token[ast->sp] = token;
	ast->stack_precedence[ast->sp] = precedence;
	ast->sp++;
}

void ast_pop(ast_context_t * ast)
{
	ast->sp--;
	ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
	ast->stack_state[ast->sp] = 0;
	ast->stack_entity[ast->sp] = 0;
	ast->stack_token[ast->sp] = 0;
	ast->stack_precedence[ast->sp] = 0;
}






void ast_parse(ast_context_t * ast)
{
	ast_push(ast, "ROOT", AST_STATE_ROOT, AST_TOKEN_UNKNOWN, 0);

machine_root:
	while(1)
	{
		char buf[BUFLEN];
		ast_token_t token = tokens_next(&ast->text_current, buf, BUFLEN);
		switch (token)
		{
		case AST_TOKEN_EOF: return;

		case AST_TOKEN_BLOCK_OPEN:
			ast_push(ast, "BLOCK", AST_STATE_BLOCK, token, 0);
			break;

		case AST_TOKEN_EQUAL:
			ast_push(ast, "EXPRESSION", AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_EXP_OPEN:
			ast_push(ast, "EXPRESSION", AST_STATE_EXPRESSION, token, 0);
			goto machine_expression;

		case AST_TOKEN_BLOCK_CLOSE:
			ast->sp--;
			ast->stack_state[ast->sp] = 0;

			if(ast->sp >= 1)
			{
				switch (ast->stack_state[ast->sp-1])
				{
					case AST_STATE_IF:
					case AST_STATE_ELSE:
						ast->sp--;
						ast->stack_state[ast->sp] = 0;
						break;
				}
			}

			if(ast->sp >= 1 && ast->stack_state[ast->sp-1] == AST_STATE_IFCASE)
			{
				// Peek one token forward:
				char const * p0 = ast->text_current;
				ast_token_t token1 = tokens_next(&p0, buf, BUFLEN);
				if(token1 != AST_TOKEN_ELSE)
				{
					ast->sp--;
					ast->stack_state[ast->sp] = 0;
				}
			}
			
			ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
			ast->stack_entity[ast->sp] = 0;
			break;

		case AST_TOKEN_EXP_CLOSE:
			ast_pop(ast);
			break;

		case AST_TOKEN_STATEMENT_TERMINATOR:
			ast->sp--;
			if(ast->sp >= 1 && (ast->stack_state[ast->sp] == AST_STATE_EXPRESSION))
			{
				ast->stack_state[ast->sp] = 0;
				ast->sp--;
			}
			ecs_set_scope(ast->world, ast->stack_entity[ast->sp]);
			ast->stack_state[ast->sp] = 0;
			break;

		case AST_TOKEN_IF:
			ast_push(ast, "IFCASE", AST_STATE_IFCASE, token, 0);
			ast_push(ast, "IF", AST_STATE_IF, token, 0);
			break;

		case AST_TOKEN_ELSE:
			ast_push(ast, "ELSE", AST_STATE_ELSE, token, 0);
			break;

		case AST_TOKEN_ID:
			if(ast->sp >= 1)
			{
				if ((ast->stack_state[ast->sp-1] == AST_STATE_BLOCK) || (ast->stack_state[ast->sp-1] == AST_STATE_ROOT))
				{
					ast_push(ast, "STATEMENT", AST_STATE_STATEMENT, token, 0);
				}
			}
			printf("Word %s\n", buf);
			ecs_new_entity(ast->world, buf);
			break;
		}
	}






machine_expression:
	
	ast_push(ast, "EXPRESSION_A", AST_STATE_EXPRESSION, AST_TOKEN_UNKNOWN, 0);
	ast_push(ast, "EXPRESSION_B", AST_STATE_EXPRESSION, AST_TOKEN_UNKNOWN, 0);
	while(1)
	{
		char buf[BUFLEN];
		ast_token_t token = tokens_next(&ast->text_current, buf, BUFLEN);
		switch (token)
		{
		case AST_TOKEN_EOF:
			return;

		case AST_TOKEN_DIV:
			ast_push(ast, "DIV", AST_STATE_EXPRESSION, token, tokens_precedence[token]);
			break;

		case AST_TOKEN_MUL:
			ecs_set_pair(ast->world, ast->stack_entity[ast->sp-1], EcsIdentifier, EcsName, {"Mul"});
			ast_push(ast, "MUL", AST_STATE_EXPRESSION, token, tokens_precedence[token]);
			break;

		case AST_TOKEN_ID:
			ecs_set_pair(ast->world, ast->stack_entity[ast->sp-1], EcsIdentifier, EcsName, {"IDNew"});
			ast_pop(ast);
			break;

		case AST_TOKEN_PLUS:
			ecs_set_pair(ast->world, ast->stack_entity[ast->sp-1], EcsIdentifier, EcsName, {"Plus"});
			ast_push(ast, "PLUS", AST_STATE_EXPRESSION, token, tokens_precedence[token]);
			break;

		case AST_TOKEN_STATEMENT_TERMINATOR:
			goto machine_root;
		}
	}





}