#pragma once
#include <stdint.h>

//https://github.com/nothings/stb/blob/master/stb_c_lexer.h

typedef enum
{
TOK_UNKNOWN,
TOK_EOF,
TOK_WHITESPACE,
TOK_BLOCK_OPEN,
TOK_BLOCK_CLOSE,
TOK_PAREN_OPEN,
TOK_PAREN_CLOSE,
TOK_SEMICOLON,
TOK_COMMA,
TOK_IF,
TOK_ELSE,
TOK_ELSEIF,
TOK_NUMBER,
TOK_EQUAL,
TOK_PLUS,
TOK_MINUS,
TOK_MUL,
TOK_DIV,
TOK_ID,
TOK_COMMENT_LINE,
TOK_COMMENT_OPEN,
TOK_COMMENT_CLOSE,
TOK_COUNT
} tok_t;



static int32_t tok_t_antiscope[] = 
{
	[TOK_BLOCK_OPEN] = TOK_BLOCK_CLOSE,
	[TOK_BLOCK_CLOSE] = TOK_BLOCK_OPEN,
	[TOK_PAREN_OPEN] = TOK_PAREN_CLOSE,
	[TOK_PAREN_CLOSE] = TOK_PAREN_OPEN,
	[TOK_COUNT] = 0
};

static int32_t tok_t_precedence[] = 
{
	[TOK_NUMBER] = 0,
	[TOK_ID] = 0,
	[TOK_EQUAL] = 14,
	[TOK_PLUS] = 2,
	[TOK_MINUS] = 2,
	[TOK_MUL] = 3,
	[TOK_DIV] = 3,
	[TOK_COMMA] = 15,
	[TOK_COUNT] = 0
};


char const * tok_t_tostr(tok_t token);
int32_t tok_t_tolen(tok_t token);