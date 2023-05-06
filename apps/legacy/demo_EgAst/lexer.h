#pragma once
#include <stdint.h>
#include "token.h"

typedef struct
{
	char const * text_start;
	char const * text_current;
	int32_t line;
	int32_t column;
} lexer_t;

typedef struct
{
	char const * cursor;
	int32_t length;
	int32_t line;
	int32_t column;
	tok_t tok;
} token_t;

void lexer_init(lexer_t * lexer);
void lexer_next(lexer_t * lexer, token_t * out_token);