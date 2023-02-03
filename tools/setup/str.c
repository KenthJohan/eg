#include "str.h"



int is_alpha(int c)
{
	if(('a' <= c) && (c <= 'z')){return 1;}
	if(('A' <= c) && (c <= 'Z')){return 1;}
	return 0;
}

int is_num(int c)
{
	if(('0' <= c) && (c <= '9')){return 1;}
	return 0;
}


void skip_whitespace(char const ** p)
{
	while(1)
	{
		switch ((*p)[0])
		{
		case '\n':
		case '\t':
		case ' ':
			(*p)++;
			break; //Break switch
		default:
			return; //Break loop
		}
	}
}

void skip_word(char const ** p)
{
	while(1)
	{
		char a = (*p)[0];
		if(is_alpha(a))
		{
			(*p)++;
			continue;
		};
		if(is_num(a))
		{
			(*p)++;
			continue;
		};
		return;
	}
}
