#include "tokens.h"
#include "str.h"
#include <stdint.h>
#include <flecs.h>




char const * ast_token_t_str(ast_token_t token)
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


int32_t ast_token_t_len(ast_token_t token)
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