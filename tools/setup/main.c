#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <flecs.h>

#include "eg_fs.h"
#include "EgAst.h"

void my_abort(void)
{
	printf("Aborted\n");
    exit(EXIT_FAILURE);
}








int main(int argc, char **argv)
{
	ecs_os_set_api_defaults();
    ecs_os_api_t os_api = ecs_os_api;
    os_api.abort_ = my_abort;
    ecs_os_set_api(&os_api);


	ecs_world_t * world = ecs_init();
	//ecs_log_set_level(1);
	//https://www.flecs.dev/explorer/?remote=true
    ecs_singleton_set(world, EcsRest, {0});
    ECS_IMPORT(world, EgAst);
	

    {
        ecs_entity_t node_if = ecs_new_entity(world, "if");
        ecs_entity_t node_condition = ecs_new_entity(world, "if_condition");
        ecs_entity_t node_content = ecs_new_entity(world, "node_content");
        ecs_add_pair(world, node_condition, EcsChildOf, node_if);
        ecs_add_pair(world, node_content, EcsChildOf, node_if);
    }




    while(true)
    {
		ecs_os_sleep(0, 1000000);
        ecs_progress(world, 0);
    }


    ecs_fini(world);

	return 0;
}



