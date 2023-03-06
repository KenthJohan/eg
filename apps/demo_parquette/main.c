#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "flecs.h"
#include "parquet.h"
#include "thrift_testing.h"






int main (int argc, char * argv [])
{
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsUnits);

	//thrift_testing_parquette("userdata1.parquet");
	thrift_testing_demo1();
	/*
	parquet_reader1_t reader = {0};
    parquet_read1(&reader, "userdata1.parquet");
	//https://www.flecs.dev/explorer/?remote=true
    ecs_singleton_set(world, EcsRest, {0});
	printf("Run ECS\n");
    return ecs_app_run(world, &(ecs_app_desc_t){
        .enable_rest = true
    });
	*/
}
