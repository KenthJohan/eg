#include "EgAst.h"





ECS_TAG_DECLARE(EgAstIf);
ECS_TAG_DECLARE(EgAstLeft);
ECS_TAG_DECLARE(EgAstRight);
ECS_TAG_DECLARE(EgAstCondition);
ECS_TAG_DECLARE(EgAstContent);
ECS_COMPONENT_DECLARE(EgAstCode);


void EgAstImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgAst);
	ecs_set_name_prefix(world, "EgAst");

	ECS_TAG_DEFINE(world, EgAstIf);
	ECS_TAG_DEFINE(world, EgAstLeft);
	ECS_TAG_DEFINE(world, EgAstRight);
	ECS_TAG_DEFINE(world, EgAstCondition);
	ECS_TAG_DEFINE(world, EgAstContent);
	ECS_COMPONENT_DEFINE(world, EgAstCode);

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity = ecs_id(EgAstCode),
	.members = {
	{ .name = "number", .type = ecs_id(ecs_i32_t) }
	}
	});


}



typedef enum
{
AST_STATE_IF_BEGIN,
AST_STATE_EXPRESSION,
AST_STATE_OP2,
AST_STATE_EXP_CLOSED,
AST_STATE_COUNT
} ast_state_t;


typedef enum
{
AST_TOKEN_EOF,
AST_TOKEN_WHITESPACE,
AST_TOKEN_BLOCK_OPEN,
AST_TOKEN_BLOCK_CLOSE,
AST_TOKEN_EXP_OPEN,
AST_TOKEN_EXP_CLOSE,
AST_TOKEN_NUMBER,
AST_TOKEN_COUNT
} ast_token_t;


static int mappings[AST_STATE_COUNT][AST_TOKEN_COUNT] =
{
	[AST_STATE_EXPRESSION] = 
	{
		[AST_TOKEN_NUMBER] = AST_STATE_OP2,
		[AST_TOKEN_EXP_CLOSE] = AST_STATE_EXP_CLOSED,
		[AST_STATE_EXP_CLOSED] = AST_STATE_EXP_CLOSED,
	}
};



int expect1(ast_state_t state, ast_token_t token)
{
	switch (state)
	{
	case AST_STATE_IF_BEGIN:
		switch (token)
		{
		case AST_TOKEN_EXP_OPEN: return 1;
		default: return 0;
		}

	case AST_STATE_EXPRESSION:
		switch (token)
		{
		case AST_TOKEN_EXP_OPEN:
		return 1;
		default: return 0;
		}
	
	default:
		break;
	}
}




















int get_token_length(int token)
{
	switch (token)
	{
	case EG_TOKEN_EOF:
	case EG_TOKEN_CODE_OPEN:
	case EG_TOKEN_CODE_CLOSE:
	case EG_TOKEN_EXP_OPEN:
	case EG_TOKEN_EXP_CLOSE:
		return 1;
	case EG_TOKEN_IF:
		return 2;
	}
	return 0;
}

int get_token(char const * text)
{
	switch (text[0])
	{
	case '\0': return EG_TOKEN_EOF;
	case '{': return EG_TOKEN_CODE_OPEN;
	case '}': return EG_TOKEN_CODE_CLOSE;
	case '(': return EG_TOKEN_EXP_OPEN;
	case ')': return EG_TOKEN_EXP_CLOSE;
	}
	if(ecs_os_strcmp(text, "if")){return EG_TOKEN_IF;}
	return EG_TOKEN_UNKNOWN;
}















void parse_block(ecs_world_t *world, int token)
{
	switch (token)
	{
	case EG_TOKEN_UNKNOWN:
	case EG_TOKEN_EOF:
		return;

	case EG_TOKEN_IF:
	case EG_TOKEN_CODE_OPEN:
	}
}




void eg_ast_parse(ecs_world_t *world, char const * text)
{
	char const * p = text;
	while(1)
	{
		int token = get_token(p);
		switch (token)
		{
		case EG_TOKEN_UNKNOWN:
		case EG_TOKEN_EOF:
			return;

		case EG_TOKEN_WHITESPACE:
			p++;
			continue;

		case EG_TOKEN_EXP_OPEN:
			p++;
			continue;
		}
		p += get_token_length(token);
	}
}