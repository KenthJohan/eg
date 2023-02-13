#include "token.h"
#include <stdint.h>
#include <flecs.h>




char const * tok_t_tostr(tok_t token)
{
	switch (token)
	{
	case TOK_EOF: return "EOF";
	case TOK_PAREN_OPEN: return "PAREN_OPEN";
	case TOK_PAREN_CLOSE: return "PAREN_CLOSE";
	case TOK_BLOCK_OPEN: return "BLOCK_OPEN";
	case TOK_BLOCK_CLOSE: return "BLOCK_CLOSE";
	case TOK_IF: return "IF";
	case TOK_SEMICOLON: return "SEMICOLON";
	case TOK_COMMA: return "TOK_COMMA";
	case TOK_ELSE: return "ELSE";
	case TOK_ELSEIF: return "ELSEIF";
	case TOK_EQUAL: return "EQUAL";
	case TOK_PLUS: return "PLUS";
	case TOK_MINUS: return "MINUS";
	case TOK_MUL: return "MUL";
	case TOK_ID: return "ID";
	case TOK_COMMENT_LINE: return "COMMENT_LINE";
	case TOK_COMMENT_OPEN: return "COMMENT_OPEN";
	case TOK_COMMENT_CLOSE: return "COMMENT_CLOSE";
	default: return "UNKNOWN";
	}
}


int32_t tok_t_tolen(tok_t token)
{
	switch (token)
	{
	case TOK_EOF:
	case TOK_PAREN_OPEN:
	case TOK_PAREN_CLOSE:
	case TOK_BLOCK_OPEN:
	case TOK_BLOCK_CLOSE:
	case TOK_SEMICOLON:
	case TOK_EQUAL:
		return 1;
	case TOK_COMMENT_LINE:
	case TOK_COMMENT_OPEN:
	case TOK_COMMENT_CLOSE:
	case TOK_IF:
		return 2;
	case TOK_ELSE:
		return 4;
	case TOK_ELSEIF:
		return 6;
	default:
		return 0;
	}
}