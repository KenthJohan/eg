#include "eg_template.h"
#include "eg_fs.h"
#include "asts.h"


void load1(ecs_world_t * world, char const * filename)
{
	char * content;
	eg_fs_readfile(&content, filename);
	printf("content: %s\n", content);
	ast_context_t ast = {0};
	ast_parse(world, &ast, content);
}