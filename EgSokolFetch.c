#include "EgSokolFetch.h"


#include "EgResources.h"
#include "EgQuantities.h"

#include "sokol/sokol_fetch.h"

#include "eg_basics.h"

static void fetch_callback(const sfetch_response_t* response)
{
	EgWorldEntity * usr = response->user_data;
	ecs_world_t *async = ecs_async_stage_new(usr->world);
	ecs_remove(async, usr->entity, EgUpdating);
	ecs_async_stage_free(async);
/*
	if (response->fetched)
	{
		int png_width = 0;
		int png_height = 0;
		int num_channels = 0;
		const int desired_channels = 4;
		stbi_uc* pixels = stbi_load_from_memory(
		response->buffer_ptr,
		(int)response->fetched_size,
		&png_width, &png_height,
		&num_channels, desired_channels);
		if (pixels)
		{
			sg_init_image(g_state.bind.fs_images[SLOT_tex], &(sg_image_desc){
			.width = png_width,
			.height = png_height,
			.pixel_format = SG_PIXELFORMAT_RGBA8,
			.min_filter = SG_FILTER_LINEAR,
			.mag_filter = SG_FILTER_LINEAR,
			.data.subimage[0][0] =
			{
			.ptr = pixels,
			.size = (size_t)(png_width * png_height * 4),
			}
			});
			stbi_image_free(pixels);
		}
	}
	else if (response->failed)
	{
		g_state.pass_action = ((sg_pass_action) {.colors[0] = { .action = SG_ACTION_CLEAR, .value = { 1.0f, 0.0f, 0.0f, 1.0f } }});
	}
	*/
}

static uint8_t file_buffer[256 * 1024];
static void System_Update(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	EgPath *path = ecs_term(it, EgPath, 1);
	EgImage *image = ecs_term(it, EgImage, 2);
	EgWorldEntity usr;
	usr.world = it->world;
	for (int i = 0; i < it->count; i ++)
	{
		char const * p = path[i].value;
		ecs_remove(it->world, it->entities[i], EgUpdate);
		ecs_add(it->world, it->entities[i], EgUpdating);
		usr.entity = it->entities[i];
		sfetch_send(&(sfetch_request_t){
		.path = p,
		.callback = fetch_callback,
		.buffer_ptr = file_buffer,
		.buffer_size = sizeof(file_buffer),
		//.user_data_ptr = &usr,
		//.user_data_size = sizeof(EgWorldEntity)
		});
	}
}


void EgSokolFetchImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSokolFetch);
	ECS_IMPORT(world, EgResources);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");


	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.terms = {
	{ .id = ecs_id(EgPath), .inout = EcsIn},
	{ .id = ecs_id(EgImage), .inout = EcsOut},
	{ .id = ecs_id(EgUpdate)}
	},
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Update
	});
}
