#include "flecs.h"
#include "sokol_app.h"
#include "test.h"



sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    ecs_world_t *world = ecs_init();
    //https://www.flecs.dev/explorer/?remote=true
    ecs_set(world, EcsWorld, EcsRest, {.port = 0});
    return (sapp_desc){
        .user_data = world,
        .init_cb = init,
        .frame_userdata_cb = frame,
        .cleanup_cb = cleanup,
        .window_title = "Primitives (Sokol GP)",
        .sample_count = 4,
    };
}