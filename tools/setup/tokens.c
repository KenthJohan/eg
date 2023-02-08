#include "tokens.h"
#include "str.h"
#include <stdint.h>
#include <flecs.h>




char const * ast_get_tokenstr(ast_token_t token)
{
	switch (token)
	{
	case AST_TOKEN_EOF: return "EOF";
	case AST_TOKEN_EXP_OPEN: return "EXP_OPEN";
	case AST_TOKEN_EXP_CLOSE: return "EXP_CLOSE";
	case AST_TOKEN_BLOCK_OPEN: return "BLOCK_OPEN";
	case AST_TOKEN_BLOCK_CLOSE: return "BLOCK_CLOSE";
	case AST_TOKEN_IF: return "IF";
	case AST_TOKEN_STATEMENT_TERMINATOR: return "STATEMENT_TERMINATOR";
	case AST_TOKEN_ELSE: return "ELSE";
	case AST_TOKEN_ELSEIF: return "ELSEIF";
	case AST_TOKEN_EQUAL: return "EQUAL";
	case AST_TOKEN_PLUS: return "PLUS";
	case AST_TOKEN_MUL: return "MUL";
	case AST_TOKEN_ID: return "ID";
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
	case AST_TOKEN_EQUAL:
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


ast_token_t tokens_next(char const ** out_p, char buf[], int32_t buflen)
{
	skip_whitespace(out_p);
	ast_token_t token = AST_TOKEN_UNKNOWN;
	char const * p = (*out_p);
	switch (p[0])
	{
	case '\0': return AST_TOKEN_EOF;
	case '(': (*out_p)++; return AST_TOKEN_EXP_OPEN;
	case ')': (*out_p)++; return AST_TOKEN_EXP_CLOSE;
	case '{': (*out_p)++; return AST_TOKEN_BLOCK_OPEN;
	case '}': (*out_p)++; return AST_TOKEN_BLOCK_CLOSE;
	case ';': (*out_p)++; return AST_TOKEN_STATEMENT_TERMINATOR;
	case '=': (*out_p)++; return AST_TOKEN_EQUAL;
	case '+': (*out_p)++; return AST_TOKEN_PLUS;
	case '-': (*out_p)++; return AST_TOKEN_MINUS;
	case '/': (*out_p)++; return AST_TOKEN_DIV;
	case '*': (*out_p)++; return AST_TOKEN_MUL;
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
		return token;
	}

	token = AST_TOKEN_ID;
	skip_word(out_p);

	if(buf)
	{
		int32_t n = (int)((*out_p) - p);
		n = n > buflen ? buflen : n;
		ecs_os_memcpy_n(buf, p, char, n);
		buf[n] = '\0'; // Null terminate
	}

	return token;
}
