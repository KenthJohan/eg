#include "lexer.h"
#include <flecs.h>

bool is_symbol_start(int c)
{
	if(c == '_'){return true;}
	if(c >= 'a' && c <= 'z'){return true;}
	if(c >= 'A' && c <= 'Z'){return true;}
	return false;
}


int is_symbol(int c)
{
	if(c == '_'){return true;}
	if(c >= 'a' && c <= 'z'){return true;}
	if(c >= 'A' && c <= 'Z'){return true;}
	if(c >= '0' && c <= '9'){return true;}
	return false;
}


tok_t lexer_one(int c)
{
	switch (c)
	{
	case '\0': return TOK_EOF;
	case '(': return TOK_PAREN_OPEN;
	case ')': return TOK_PAREN_CLOSE;
	case '{': return TOK_BLOCK_OPEN;
	case '}': return TOK_BLOCK_CLOSE;
	case ';': return TOK_SEMICOLON;
	case '=': return TOK_EQUAL;
	case '+': return TOK_PLUS;
	case '-': return TOK_MINUS;
	case '/': return TOK_DIV;
	case '*': return TOK_MUL;
	default: return TOK_UNKNOWN;
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
	tok_t t = lexer_one(lexer->text_current[0]);
	if(t != TOK_UNKNOWN)
	{
		out_token->cursor = lexer->text_current;
		out_token->length = 1;
		out_token->column = lexer->column;
		out_token->line = lexer->line;
		out_token->tok = t;
		lexer->text_current += out_token->length;
		lexer->column += out_token->length;
		return;
	}
	else if(ecs_os_strncmp(lexer->text_current, "if ", 3) == 0){t = TOK_IF;}
	else if(ecs_os_strncmp(lexer->text_current, "if(", 3) == 0){t = TOK_IF;}
	else if(ecs_os_strncmp(lexer->text_current, "if\n", 3) == 0){t = TOK_IF;}
	else if(ecs_os_strncmp(lexer->text_current, "if\t", 3) == 0){t = TOK_IF;}
	else if(ecs_os_strncmp(lexer->text_current, "else ", 5) == 0){t = TOK_ELSE;}
	else if(ecs_os_strncmp(lexer->text_current, "else{", 5) == 0){t = TOK_ELSE;}
	else if(ecs_os_strncmp(lexer->text_current, "else\n", 5) == 0){t = TOK_ELSE;}
	else if(ecs_os_strncmp(lexer->text_current, "else\t", 5) == 0){t = TOK_ELSE;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif ", 7) == 0){t = TOK_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif(", 7) == 0){t = TOK_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif{", 7) == 0){t = TOK_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif\n", 7) == 0){t = TOK_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "elseif\t", 7) == 0){t = TOK_ELSEIF;}
	else if(ecs_os_strncmp(lexer->text_current, "//", 2) == 0){t = TOK_COMMENT_LINE;}
	else if(ecs_os_strncmp(lexer->text_current, "/*", 2) == 0){t = TOK_COMMENT_OPEN;}
	else if(ecs_os_strncmp(lexer->text_current, "*/", 2) == 0){t = TOK_COMMENT_CLOSE;}

	if(t != TOK_UNKNOWN)
	{
		out_token->cursor = lexer->text_current;
		out_token->length = tok_t_tolen(t);
		out_token->column = lexer->column;
		out_token->line = lexer->line;
		out_token->tok = t;
		lexer->text_current += out_token->length;
		lexer->column += out_token->length;
		return;
	}

	if(is_symbol_start(lexer->text_current[0]))
	{
		out_token->length = 0;
		out_token->tok = TOK_ID;
		out_token->cursor = lexer->text_current;
		while(1)
		{
			if(is_symbol(lexer->text_current[0]) == 0){break;}
			out_token->length++;
			lexer->text_current++;
		}
	}





}