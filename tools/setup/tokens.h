#pragma once
#include <stdint.h>

//https://github.com/nothings/stb/blob/master/stb_c_lexer.h

typedef enum
{
AST_TOKEN_UNKNOWN,
AST_TOKEN_EOF,
AST_TOKEN_WHITESPACE,
AST_TOKEN_BLOCK_OPEN,
AST_TOKEN_BLOCK_CLOSE,
AST_TOKEN_EXP_OPEN,
AST_TOKEN_EXP_CLOSE,
AST_TOKEN_STATEMENT_TERMINATOR,
AST_TOKEN_IF,
AST_TOKEN_ELSE,
AST_TOKEN_ELSEIF,
AST_TOKEN_NUMBER,
AST_TOKEN_EQUAL,
AST_TOKEN_PLUS,
AST_TOKEN_MINUS,
AST_TOKEN_MUL,
AST_TOKEN_DIV,
AST_TOKEN_ID,
AST_TOKEN_COUNT
} ast_token_t;



static int32_t tokens_precedence[] = 
{
	[AST_TOKEN_NUMBER] = 0,
	[AST_TOKEN_ID] = 0,
	[AST_TOKEN_EQUAL] = 14,
	[AST_TOKEN_PLUS] = 2,
	[AST_TOKEN_MINUS] = 2,
	[AST_TOKEN_MUL] = 3,
	[AST_TOKEN_DIV] = 3,
	[AST_TOKEN_COUNT] = 0
};




ast_token_t tokens_next(char const ** out_p, char buf[], int32_t buflen);
char const * ast_get_tokenstr(ast_token_t token);
int32_t ast_get_tokenlen(int32_t token);