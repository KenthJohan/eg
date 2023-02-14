#include "EgAst.h"
#include "asts.h"
#include <stdio.h>


#define ASSERT(c) if (!(c)) __builtin_trap()


ECS_TAG_DECLARE(EgAstEroot);
ECS_TAG_DECLARE(EgAstIf);
ECS_TAG_DECLARE(EgAstLeft);
ECS_TAG_DECLARE(EgAstRight);
ECS_TAG_DECLARE(EgAstCondition);
ECS_TAG_DECLARE(EgAstContent);
ECS_TAG_DECLARE(EgAstNumber);
ECS_COMPONENT_DECLARE(EgAstCode);



void EgAstImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgAst);
	ecs_set_name_prefix(world, "EgAst");

	ECS_TAG_DEFINE(world, EgAstEroot);
	ECS_TAG_DEFINE(world, EgAstIf);
	ECS_TAG_DEFINE(world, EgAstLeft);
	ECS_TAG_DEFINE(world, EgAstRight);
	ECS_TAG_DEFINE(world, EgAstCondition);
	ECS_TAG_DEFINE(world, EgAstContent);
	ECS_TAG_DEFINE(world, EgAstNumber);
	ECS_COMPONENT_DEFINE(world, EgAstCode);

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity = ecs_id(EgAstCode),
	.members = {
	{ .name = "number", .type = ecs_id(ecs_i32_t) }
	}
	});


}




char const * tok_t_tocolor(tok_t token)
{
	switch (token)
	{
	case TOK_EOF:           return "#111111";
	case TOK_PAREN_OPEN:    return "#111111";
	case TOK_PAREN_CLOSE:   return "#111111";
	case TOK_BLOCK_OPEN:    return "#111111";
	case TOK_BLOCK_CLOSE:   return "#111111";
	case TOK_IF:            return "#111111";
	case TOK_SEMICOLON:     return "#111111";
	case TOK_ELSE:          return "#111111";
	case TOK_ELSEIF:        return "#111111";
	case TOK_EQUAL:         return "#0000FF";
	case TOK_PLUS:          return "#0000FF";
	case TOK_MINUS:         return "#0000FF";
	case TOK_MUL:           return "#0000FF";
	case TOK_ID:            return "#FF0000";
	case TOK_COMMENT_LINE:  return "#111111";
	case TOK_COMMENT_OPEN:  return "#111111";
	case TOK_COMMENT_CLOSE: return "#111111";
	default:                return "#111111";
	}
}



#define BUFLEN 128
ecs_entity_t EgAst_newent(ecs_world_t * world, token_t * token, ast_parse_t parse)
{
	ASSERT(world);
	ASSERT(token);

	char buf[BUFLEN];
	ecs_entity_t e = ecs_new_entity(world, 0);
	switch (token->tok)
	{
	case TOK_ID:{
		int32_t l = token->length;
		l = l < BUFLEN ? l : BUFLEN-1;
		memcpy(buf, token->cursor, l);
		buf[l] = '\0';
		break;}
	
	default:
		snprintf(buf, BUFLEN, "%s", tok_t_tostr(token->tok));
		break;
	}
	ecs_doc_set_name(world, e, buf);
	ecs_doc_set_color(world, e, tok_t_tocolor(token->tok));
	printf(AST_TITLE_FORMAT ECS_GREY"%s"ECS_NORMAL" > "ECS_BLUE"%s"ECS_NORMAL"\n", "[New]",
		ecs_doc_get_name(world, ecs_get_scope(world)),  
		ecs_doc_get_name(world, e));

	switch (parse)
	{
	case AST_PARSE_EROOT:
		ecs_doc_set_color(world, e, "#0044FF");
		ecs_add(world, e, EgAstEroot);
		break;
	}
	return e;
}














