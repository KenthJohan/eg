#include "lexer.h"
#include <ctype.h>
#include <flecs.h>

int is_symbol_start(int c)
{
	return isalpha(c) || (c == '_');
}


int is_symbol(int c)
{
	return isalnum(c) || (c == '_');
}





token_constant_t lexer_one(int c)
{
	switch (c)
	{
	case '\0': return TOKEN_CONSTANT_EOF;
	case '(': return TOKEN_CONSTANT_EXP_OPEN;
	case ')': return TOKEN_CONSTANT_EXP_CLOSE;
	case '{': return TOKEN_CONSTANT_BLOCK_OPEN;
	case '}': return TOKEN_CONSTANT_BLOCK_CLOSE;
	case ';': return TOKEN_CONSTANT_STATEMENT_TERMINATOR;
	case '=': return TOKEN_CONSTANT_EQUAL;
	case '+': return TOKEN_CONSTANT_PLUS;
	case '-': return TOKEN_CONSTANT_MINUS;
	case '/': return TOKEN_CONSTANT_DIV;
	case '*': return TOKEN_CONSTANT_MUL;
	default: return TOKEN_CONSTANT_UNKNOWN;
	}
}



void lexer_skip_whitespace(char const ** p, int32_t * out_line, int32_t * out_column)
{
	while(1)
	{
		switch ((*p)[0])
		{
		case '\n':
			(*out_column) = 0;
			(*out_line)++;
			(*p)++;
			break; //Break switch

		case '\t':
			(*out_column)++;
			(*p)++;
			break; //Break switch

		case ' ':
			(*out_column)++;
			(*p)++;
			break; //Break switch

		default:
			return; //Break loop

		} // End of switch
	} // End of while loop
}



void lexer_init(lexer_t * lexer)
{
	lexer->text_start = 0;
	lexer->text_current = 0;
	lexer->column = 0;
	lexer->line = 0;
}


void lexer_next(lexer_t * lexer, token_t * out_token)
{
	lexer_skip_whitespace(&lexer->text_current, &lexer->line, &lexer->column);
	token_constant_t t = lexer_one(lexer->text_current[0]);
	if(t != TOKEN_CONSTANT_UNKNOWN)
	{
		out_token->cursor = lexer->text_current;
		out_token->length = 1;
		out_token->column = lexer->column;
		out_token->line = lexer->line;
		out_token->tokconstant = t;
		lexer->text_current += out_token->length;
		lexer->column += out_token->length;
		return;
	}
	else if(ecs_os_strncmp(lexer->text_current, "if ", 3) == 0){t = TOKEN_CONSTANT_IF;}
	else if(ecs_os_strncmp(lexer->text_current, "if(", 3) == 0){t = TOKEN_CONSTANT_IF;}
	else if(ecs_os_strncmp(lexer->text_current, "if\n", 3) == 0){t = TOKEN_CONSTANT_IF;}
	else if(ecs_os_strncmp(lexer->text_current, "if\t", 3) == 0){t = TOKEN_CONSTANT_IF;}
	else if(ecs_os_strncmp(lexer->text_current, "else ", 5) == 0){t = TOKEN_CONSTANT_ELSE;}
	else if(ecs_os_strncmp(lexer->text_current, "else{", 5) == 0){t = TOKEN_CONSTANT_ELSE;}
	else if(ecs_os_strncmp(lexer->text_current, "else\n", 5) == 0){t = TOKEN_CONSTANT_ELSE;}
	else if(ecs_os_strncmp(lexer->text_current, "else\t", 5) == 0){t = TOKEN_CONSTANT_ELSE;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif ", 7) == 0){t = TOKEN_CONSTANT_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif(", 7) == 0){t = TOKEN_CONSTANT_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif{", 7) == 0){t = TOKEN_CONSTANT_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif\n", 7) == 0){t = TOKEN_CONSTANT_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif\t", 7) == 0){t = TOKEN_CONSTANT_ELSEIF;}

	if(t != TOKEN_CONSTANT_UNKNOWN)
	{
		out_token->cursor = lexer->text_current;
		out_token->length = token_constant_t_tolen(t);
		out_token->column = lexer->column;
		out_token->line = lexer->line;
		out_token->tokconstant = t;
		lexer->text_current += out_token->length;
		lexer->column += out_token->length;
		return;
	}

	if(is_symbol_start(lexer->text_current[0]))
	{
		out_token->length = 0;
		out_token->tokconstant = TOKEN_CONSTANT_ID;
		out_token->cursor = lexer->text_current;
		while(1)
		{
			if(is_symbol(lexer->text_current[0]) == 0){break;}
			out_token->length++;
			lexer->text_current++;
		}
	}





}