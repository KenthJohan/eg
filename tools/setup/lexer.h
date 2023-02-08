#pragma once
#include <stdint.h>
#include "tokens.h"

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
	ast_token_t type;
} token_t;