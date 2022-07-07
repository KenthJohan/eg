#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>

#include "flecs.h"

#include "sokol_source.h"
#include "libs/stb/stb_image.h"

typedef struct
{
	ecs_world_t *world;
	ecs_entity_t entity;
	ecs_os_mutex_t lock;
} world_entity_t;

typedef struct
{
	stbi_uc* pixels;
	int width;
	int height;
	int channels;
} Texture;
ECS_COMPONENT_DECLARE(Texture);

typedef struct
{
	char * path;
} TextureLoader;
ECS_COMPONENT_DECLARE(TextureLoader);

static void fetch_callback(const sfetch_response_t* response)
{
	world_entity_t *we = response->user_data;
	if (response->fetched)
	{
		Texture tex;
		const int desired_channels = 4;
		tex.pixels = stbi_load_from_memory(response->buffer_ptr,(int)response->fetched_size, &tex.width, &tex.height, &tex.channels, desired_channels);
		ecs_set_ptr(we->world, we->entity, Texture, &tex);
	}
}

static void System_TextureLoader(ecs_iter_t *it)
{
	static uint8_t file_buffer[256 * 1024];
	world_entity_t we;
	we.world = it->world;
	TextureLoader *loader = ecs_term(it, TextureLoader, 1);
	for (int i = 0; i < it->count; i ++)
	{
		we.entity = ecs_new(we.world, 0);
		sfetch_send(&(sfetch_request_t){
		.path = loader[i].path,
		.callback = fetch_callback,
		.buffer_ptr = file_buffer,
		.buffer_size = sizeof(file_buffer),
		.user_data_ptr = &we,
		.user_data_size = sizeof(world_entity_t)
		});
		ecs_remove(it->world, it->entities[i], TextureLoader);
	}
}


void myabort()
{
	__debugbreak();
	raise(SIGINT);
}

// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	ecs_os_set_api_defaults();
	ecs_os_api_t os_api = ecs_os_api;
	os_api.abort_ = myabort;
	ecs_os_set_api(&os_api);

	sfetch_setup(&(sfetch_desc_t){
	.max_requests = 2,
	.num_channels = 1,
	.num_lanes = 1
	});
	ecs_log_set_level(0);
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ecs_singleton_set(world, EcsRest, {0});
	ECS_COMPONENT_DEFINE(world, Texture);
	ECS_COMPONENT_DEFINE(world, TextureLoader);

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(Texture),
	.members = {
	{ .name = "pixels", .type = ecs_id(ecs_uptr_t) },
	{ .name = "width", .type = ecs_id(ecs_i32_t) },
	{ .name = "height", .type = ecs_id(ecs_i32_t) },
	{ .name = "channels", .type = ecs_id(ecs_i32_t) }
	}});
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(TextureLoader),
	.members = {
	{ .name = "path", .type = ecs_id(ecs_string_t) }
	}});
	ECS_SYSTEM(world, System_TextureLoader, EcsOnUpdate, TextureLoader);
	//ECS_IMPORT(world, FlecsMonitor);
	//ECS_IMPORT(world, FlecsUnits);



	ecs_set(world, 0, TextureLoader, {"../eg/baboon.png"});
	ecs_set(world, 0, TextureLoader, {"../eg/tree.png"});

	while (1)
	{
		ecs_os_sleep(0,100000);
		ecs_progress(world, 0);
		sfetch_dowork();
	}

	return 0;
}
