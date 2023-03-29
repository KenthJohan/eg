#include "flecs.h"
#include "sokol_app.h"
#include "test.h"


//ECS_DECLARE(EgSappDesc);

/*
int main(int argc, char* argv[])
{
    ecs_world_t *world = ecs_init();
    (void)argc;
    (void)argv;

	//ECS_IMPORT(world, EgSapp);


    return ecs_app_run(world, &(ecs_app_desc_t){
        .target_fps = 60, 
        .enable_rest = true,
        .enable_monitor = true
    });
}
*/



sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .window_title = "Primitives (Sokol GP)",
        .sample_count = 4,
    };
}