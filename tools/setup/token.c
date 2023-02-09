#include "token.h"
#include <stdint.h>
#include <flecs.h>




char const * token_constant_t_tostr(token_constant_t token)
{
	switch (token)
	{
	case TOKEN_CONSTANT_EOF: return "EOF";
	case TOKEN_CONSTANT_EXP_OPEN: return "EXP_OPEN";
	case TOKEN_CONSTANT_EXP_CLOSE: return "EXP_CLOSE";
	case TOKEN_CONSTANT_BLOCK_OPEN: return "BLOCK_OPEN";
	case TOKEN_CONSTANT_BLOCK_CLOSE: return "BLOCK_CLOSE";
	case TOKEN_CONSTANT_IF: return "IF";
	case TOKEN_CONSTANT_STATEMENT_TERMINATOR: return "STATEMENT_TERMINATOR";
	case TOKEN_CONSTANT_ELSE: return "ELSE";
	case TOKEN_CONSTANT_ELSEIF: return "ELSEIF";
	case TOKEN_CONSTANT_EQUAL: return "EQUAL";
	case TOKEN_CONSTANT_PLUS: return "PLUS";
	case TOKEN_CONSTANT_MUL: return "MUL";
	case TOKEN_CONSTANT_ID: return "ID";
	case TOKEN_CONSTANT_COMMENT_LINE: return "COMMENT_LINE";
	case TOKEN_CONSTANT_COMMENT_OPEN: return "COMMENT_OPEN";
	case TOKEN_CONSTANT_COMMENT_CLOSE: return "COMMENT_CLOSE";
	default: return "UNKNOWN";
	}
}


int32_t token_constant_t_tolen(token_constant_t token)
{
	switch (token)
	{
	case TOKEN_CONSTANT_EOF:
	case TOKEN_CONSTANT_EXP_OPEN:
	case TOKEN_CONSTANT_EXP_CLOSE:
	case TOKEN_CONSTANT_BLOCK_OPEN:
	case TOKEN_CONSTANT_BLOCK_CLOSE:
	case TOKEN_CONSTANT_STATEMENT_TERMINATOR:
	case TOKEN_CONSTANT_EQUAL:
		return 1;
	case TOKEN_CONSTANT_COMMENT_LINE:
	case TOKEN_CONSTANT_COMMENT_OPEN:
	case TOKEN_CONSTANT_COMMENT_CLOSE:
	case TOKEN_CONSTANT_IF:
		return 2;
	case TOKEN_CONSTANT_ELSE:
		return 4;
	case TOKEN_CONSTANT_ELSEIF:
		return 6;
	default:
		return 0;
	}
}