#include "asts.h"
#include "str.h"
#include "tokens.h"
#include <stdio.h>

#define BUFLEN 128

void ast_push(ecs_world_t * world, ast_context_t * ast, char const * name, ast_state_t state)
{
	ast->genid++;
	char buf[BUFLEN];
	snprintf(buf, BUFLEN, "%s%i", name, ast->genid);
	ecs_entity_t e = ecs_new_entity(world, buf);
	ast->stack1[ast->sp] = ecs_set_scope(world, e);
	ast->stack[ast->sp] = state;
	ast->sp++;
}

void ast_pop(ecs_world_t * world, ast_context_t * ast)
{
	ast->sp--;
	ecs_set_scope(world, ast->stack1[ast->sp]);
	ast->stack[ast->sp] = 0;
	ast->stack1[ast->sp] = 0;
}



void get_word(char buf[], int32_t n, char const ** p)
{
	/*
	char * p0 = p;
	skip_word(&p);
	n = (int)(p-p0);
	ecs_os_memcpy_n(buf, p0, char, n);
	buf[n] = '\0'; // Null terminate
	*/
}






void ast_parsex(ecs_world_t * world, ast_context_t * ast)
{

}







void ast_parse(ecs_world_t * world, ast_context_t * ast)
{
	int n;
	ecs_entity_t e;
	ast_token_t token;
	char buf[BUFLEN];

	ecs_entity_t current;


	ast_push(world, ast, "ROOT", AST_STATE_ROOT);

	/*
	e = ecs_new_entity(world, "ROOT");
	ast->stack1[ast->sp] = ecs_set_scope(world, e);
	ast->stack[ast->sp] = AST_STATE_ROOT;
	ast->sp++;
	*/

	while(1)
	{
		token = ast_get_token(&ast->text_current, buf, BUFLEN);
		
		
		switch (token)
		{
		case AST_TOKEN_EOF: return;

		case AST_TOKEN_BLOCK_OPEN:
			ast_push(world, ast, "BLOCK", AST_STATE_BLOCK);
			break;

		case AST_TOKEN_EQUAL:
			ast_push(world, ast, "EXPRESSION", AST_STATE_EXPRESSION);
			break;

		case AST_TOKEN_EXP_OPEN:
			ast_push(world, ast, "EXPRESSION", AST_STATE_EXPRESSION);
			break;

		case AST_TOKEN_BLOCK_CLOSE:
			ast->sp--;
			ast->stack[ast->sp] = 0;

			if(ast->sp >= 1)
			{
				switch (ast->stack[ast->sp-1])
				{
					case AST_STATE_IF:
					case AST_STATE_ELSE:
						ast->sp--;
						ast->stack[ast->sp] = 0;
						break;
				}
			}

			if(ast->sp >= 1 && ast->stack[ast->sp-1] == AST_STATE_IFCASE)
			{
				// Peek one token forward:
				char const * p0 = ast->text_current;
				ast_token_t token1 = ast_get_token(&p0, buf, BUFLEN);
				if(token1 != AST_TOKEN_ELSE)
				{
					ast->sp--;
					ast->stack[ast->sp] = 0;
				}
			}
			
			ecs_set_scope(world, ast->stack1[ast->sp]);
			ast->stack1[ast->sp] = 0;
			break;

		case AST_TOKEN_EXP_CLOSE:
			ast_pop(world, ast);
			break;

		case AST_TOKEN_STATEMENT_TERMINATOR:
			ast->sp--;
			if(ast->sp >= 1 && (ast->stack[ast->sp] == AST_STATE_EXPRESSION))
			{
				ast->stack[ast->sp] = 0;
				ast->sp--;
			}
			ecs_set_scope(world, ast->stack1[ast->sp]);
			ast->stack[ast->sp] = 0;
			break;

		case AST_TOKEN_IF:
			ast_push(world, ast, "IFCASE", AST_STATE_IFCASE);
			ast_push(world, ast, "IF", AST_STATE_IF);
			break;

		case AST_TOKEN_ELSE:
			ast_push(world, ast, "ELSE", AST_STATE_ELSE);
			break;

		case AST_TOKEN_ID:
			if(ast->sp >= 1)
			{
				if ((ast->stack[ast->sp-1] == AST_STATE_BLOCK) || (ast->stack[ast->sp-1] == AST_STATE_ROOT))
				{
					ast_push(world, ast, "STATEMENT", AST_STATE_STATEMENT);
				}
			}
			printf("Word %s\n", buf);
			e = ecs_new_entity(world, buf);
			break;

		}
	}
}