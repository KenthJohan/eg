#include "asts.h"
#include <stdio.h>




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


int32_t ast_get_token(char const * text)
{
	char const * p = text;
	switch (p[0])
	{
	case '\0': return AST_TOKEN_EOF;
	case '(': return AST_TOKEN_EXP_OPEN;
	case ')': return AST_TOKEN_EXP_CLOSE;
	case '{': return AST_TOKEN_BLOCK_OPEN;
	case '}': return AST_TOKEN_BLOCK_CLOSE;
	case ';': return AST_TOKEN_STATEMENT_TERMINATOR;
	}
	if(ecs_os_strncmp(p, "if ", 3) == 0){return AST_TOKEN_IF;}
	if(ecs_os_strncmp(p, "if(", 3) == 0){return AST_TOKEN_IF;}
	if(ecs_os_strncmp(p, "if\n", 3) == 0){return AST_TOKEN_IF;}
	if(ecs_os_strncmp(p, "if\t", 3) == 0){return AST_TOKEN_IF;}

	if(ecs_os_strncmp(p, "else ", 5) == 0){return AST_TOKEN_ELSE;}
	if(ecs_os_strncmp(p, "else{", 5) == 0){return AST_TOKEN_ELSE;}
	if(ecs_os_strncmp(p, "else\n", 5) == 0){return AST_TOKEN_ELSE;}
	if(ecs_os_strncmp(p, "else\t", 5) == 0){return AST_TOKEN_ELSE;}

	if(ecs_os_strncmp(p, "elseif ", 7) == 0){return AST_TOKEN_ELSEIF;}
	if(ecs_os_strncmp(p, "elseif{", 7) == 0){return AST_TOKEN_ELSEIF;}
	if(ecs_os_strncmp(p, "elseif\n", 7) == 0){return AST_TOKEN_ELSEIF;}
	if(ecs_os_strncmp(p, "elseif\t", 7) == 0){return AST_TOKEN_ELSEIF;}
	return AST_TOKEN_UNKNOWN;
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


void ast_parse(ecs_world_t * world, ast_context_t * ast, char const * text)
{
	char buf[128];
	char const * p = text;
	char const * p0;
	int n;
	ecs_entity_t e;
	ast_token_t token;
	int i = 0;

	e = ecs_new_entity(world, "ROOT");
	ast->stack1[ast->sp] = ecs_set_scope(world, e);
	ast->stack[ast->sp] = AST_STATE_ROOT;
	ast->sp++;

	while(1)
	{
		skip_whitespace(&p);
		token = ast_get_token(p);
		i++;
		switch (token)
		{
		case AST_TOKEN_EOF: return;

		case AST_TOKEN_BLOCK_OPEN:
			e = ecs_new_entity(world, "BLOCK");
			ast->stack1[ast->sp] = ecs_set_scope(world, e);
			ast->stack[ast->sp] = AST_STATE_BLOCK;
			ast->sp++;
			break;


		case AST_TOKEN_EXP_OPEN:
			e = ecs_new_entity(world, "EXPRESSION");
			ast->stack1[ast->sp] = ecs_set_scope(world, e);
			ast->stack[ast->sp] = AST_STATE_EXPRESSION;
			ast->sp++;
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
				p0 = p + ast_get_tokenlen(token);
				skip_whitespace(&p0);
				ast_token_t token1 = ast_get_token(p0);
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
			ast->sp--;
			ast->stack[ast->sp] = 0;
			ecs_set_scope(world, ast->stack1[ast->sp]);
			ast->stack1[ast->sp] = 0;
			break;

		case AST_TOKEN_STATEMENT_TERMINATOR:
			ast->sp--;
			ast->stack[ast->sp] = 0;
			ecs_set_scope(world, ast->stack1[ast->sp]);
			ast->stack1[ast->sp] = 0;
			break;

		case AST_TOKEN_IF:
			e = ecs_new_entity(world, "IFCASE");
			ast->stack1[ast->sp] = ecs_set_scope(world, e);
			ast->stack[ast->sp] = AST_STATE_IFCASE;
			ast->sp++;
			e = ecs_new_entity(world, "IF");
			ast->stack1[ast->sp] = ecs_set_scope(world, e);
			ast->stack[ast->sp] = AST_STATE_IF;
			ast->sp++;
			break;

		case AST_TOKEN_ELSE:
			snprintf(buf, 128, "%s%i", "ELSE", i);
			e = ecs_new_entity(world, buf);
			ast->stack1[ast->sp] = ecs_set_scope(world, e);
			ast->stack[ast->sp] = AST_STATE_ELSE;
			ast->sp++;
			break;

		default:
			if(ast->sp >= 1)
			{
				if ((ast->stack[ast->sp-1] == AST_STATE_BLOCK) || (ast->stack[ast->sp-1] == AST_STATE_ROOT))
				{
					e = ecs_new_entity(world, "STATEMENT");
					ast->stack1[ast->sp] = ecs_set_scope(world, e);
					ast->stack[ast->sp] = AST_STATE_STATEMENT;
					ast->sp++;
				}
			}
			p0 = p;
			skip_word(&p);
			n = (int)(p-p0);
			ecs_os_memcpy_n(buf, p0, char, n);
			buf[n] = '\0'; // Null terminate
			printf("Word %s\n", buf);
			e = ecs_new_entity(world, buf);
			break;

		}
		p += ast_get_tokenlen(token);
	}
}