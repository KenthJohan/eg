#include "tokens.h"
#include <stdint.h>
#include <flecs.h>

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
