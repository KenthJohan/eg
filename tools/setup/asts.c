#include "asts.h"
#include <stdio.h>




int is_alpha(int c)
{
	if(('a' <= c) && (c <= 'z')){return 1;}
	if(('A' <= c) && (c <= 'Z')){return 1;}
	return 0;
}

int is_num(int c)
{
	if(('0' <= c) && (c <= '9')){return 1;}
	return 0;
}


void skip_whitespace(char const ** p)
{
	while(1)
	{
		switch ((*p)[0])
		{
		case '\n':
		case '\t':
		case ' ':
			(*p)++;
			break; //Break switch
		default:
			return; //Break loop
		}
	}
}

void skip_word(char const ** p)
{
	while(1)
	{
		char a = (*p)[0];
		if(is_alpha(a))
		{
			(*p)++;
			continue;
		};
		if(is_num(a))
		{
			(*p)++;
			continue;
		};
		return;
	}
}










char const * ast_get_tokenstr(ast_token_t token)
{
	switch (token)
	{
	case AST_TOKEN_EOF: return "AST_TOKEN_EOF";
	case AST_TOKEN_EXP_OPEN: return "AST_TOKEN_EXP_OPEN";
	case AST_TOKEN_EXP_CLOSE: return "AST_TOKEN_EXP_CLOSE";
	case AST_TOKEN_BLOCK_OPEN: return "AST_TOKEN_BLOCK_OPEN";
	case AST_TOKEN_BLOCK_CLOSE: return "AST_TOKEN_BLOCK_CLOSE";
	case AST_TOKEN_IF: return "AST_TOKEN_IF";
	case AST_TOKEN_STATEMENT_TERMINATOR: return "AST_TOKEN_STATEMENT_TERMINATOR";
	case AST_TOKEN_ELSE: return "AST_TOKEN_ELSE";
	case AST_TOKEN_ELSEIF: return "AST_TOKEN_ELSEIF";
	default: return "UNKNOWN";
	}
}


int32_t ast_get_tokenlen(int32_t token)
{
	switch (token)
	{
	case AST_TOKEN_EOF:
	case AST_TOKEN_EXP_OPEN:
	case AST_TOKEN_EXP_CLOSE:
	case AST_TOKEN_BLOCK_OPEN:
	case AST_TOKEN_BLOCK_CLOSE:
	case AST_TOKEN_STATEMENT_TERMINATOR:
		return 1;
	case AST_TOKEN_IF:
		return 2;
	case AST_TOKEN_ELSE:
		return 4;
	case AST_TOKEN_ELSEIF:
		return 6;
	default:
		return 0;
	}
}


ast_token_t ast_get_token(char const ** out_p, char buf[], int32_t buflen)
{
	skip_whitespace(out_p);
	ast_token_t token = AST_TOKEN_UNKNOWN;
	char const * p = (*out_p);
	switch (p[0])
	{
	case '\0': token = AST_TOKEN_EOF;break;
	case '(': token = AST_TOKEN_EXP_OPEN;break;
	case ')': token = AST_TOKEN_EXP_CLOSE;break;
	case '{': token = AST_TOKEN_BLOCK_OPEN;break;
	case '}': token = AST_TOKEN_BLOCK_CLOSE;break;
	case ';': token = AST_TOKEN_STATEMENT_TERMINATOR;break;
	}

	if(0){}
	else if(ecs_os_strncmp(p, "if ", 3) == 0){token = AST_TOKEN_IF;}
	else if(ecs_os_strncmp(p, "if(", 3) == 0){token = AST_TOKEN_IF;}
	else if(ecs_os_strncmp(p, "if\n", 3) == 0){token = AST_TOKEN_IF;}
	else if(ecs_os_strncmp(p, "if\t", 3) == 0){token = AST_TOKEN_IF;}
	else if(ecs_os_strncmp(p, "else ", 5) == 0){token = AST_TOKEN_ELSE;}
	else if(ecs_os_strncmp(p, "else{", 5) == 0){token = AST_TOKEN_ELSE;}
	else if(ecs_os_strncmp(p, "else\n", 5) == 0){token = AST_TOKEN_ELSE;}
	else if(ecs_os_strncmp(p, "else\t", 5) == 0){token = AST_TOKEN_ELSE;}
	else if(ecs_os_strncmp(p, "elseif ", 7) == 0){token = AST_TOKEN_ELSEIF;}
	else if(ecs_os_strncmp(p, "elseif{", 7) == 0){token = AST_TOKEN_ELSEIF;}
	else if(ecs_os_strncmp(p, "elseif\n", 7) == 0){token = AST_TOKEN_ELSEIF;}
	else if(ecs_os_strncmp(p, "elseif\t", 7) == 0){token = AST_TOKEN_ELSEIF;}


	if(token != AST_TOKEN_UNKNOWN)
	{
		(*out_p) += ast_get_tokenlen(token);
	}
	else
	{
		skip_word(out_p);
		int32_t n = (int)((*out_p) - p);
		n = n > buflen ? buflen : n;
		ecs_os_memcpy_n(buf, p, char, n);
		buf[n] = '\0'; // Null terminate
		token = AST_TOKEN_ID;
	}
	return token;
}









void ast_push(ecs_world_t * world, ast_context_t * ast, char const * name, ast_state_t state)
{
	ecs_entity_t e = ecs_new_entity(world, name);
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





#define BUFLEN 128
void ast_parse(ecs_world_t * world, ast_context_t * ast, char const * text)
{
	char buf[BUFLEN];
	char const * p = text;
	int n;
	ecs_entity_t e;
	ast_token_t token;
	int i = 0;

	ast_push(world, ast, "ROOT", AST_STATE_ROOT);

	/*
	e = ecs_new_entity(world, "ROOT");
	ast->stack1[ast->sp] = ecs_set_scope(world, e);
	ast->stack[ast->sp] = AST_STATE_ROOT;
	ast->sp++;
	*/

	while(1)
	{
		i++;

		token = ast_get_token(&p, buf, BUFLEN);
		
		
		switch (token)
		{
		case AST_TOKEN_EOF: return;

		case AST_TOKEN_BLOCK_OPEN:
			ast_push(world, ast, "BLOCK", AST_STATE_BLOCK);
			break;


		case AST_TOKEN_EXP_OPEN:
			ast_push(world, ast, "EXPRESSION", AST_STATE_EXPRESSION);
			break;

		case AST_TOKEN_BLOCK_CLOSE:
			ast->sp--;

			if(ast->sp >= 1)
			{
				switch (ast->stack[ast->sp-1])
				{
					case AST_STATE_IF:
					case AST_STATE_ELSE:
						ast->stack[ast->sp] = 0;
						ast->sp--;
						break;
				}
			}

			if(ast->sp >= 1 && ast->stack[ast->sp-1] == AST_STATE_IFCASE)
			{
				// Peek one token forward:
				char const * p0 = p;
				ast_token_t token1 = ast_get_token(&p0, buf, BUFLEN);
				if(token1 != AST_TOKEN_ELSE)
				{
					ast->stack[ast->sp] = 0;
					ast->sp--;
				}
			}

			ast->stack[ast->sp] = 0;
			ecs_set_scope(world, ast->stack1[ast->sp]);
			ast->stack1[ast->sp] = 0;
			break;

		case AST_TOKEN_EXP_CLOSE:
			ast_pop(world, ast);
			break;

		case AST_TOKEN_STATEMENT_TERMINATOR:
			ast_pop(world, ast);
			break;

		case AST_TOKEN_IF:
			ast_push(world, ast, "IFCASE", AST_STATE_IFCASE);
			ast_push(world, ast, "IF", AST_STATE_IF);
			break;

		case AST_TOKEN_ELSE:
			snprintf(buf, 128, "%s%i", "ELSE", i);
			ast_push(world, ast, buf, AST_STATE_ELSE);
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