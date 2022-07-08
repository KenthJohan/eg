#include "EgSokolFetch.h"
#include "EgResources.h"
#include "EgQuantities.h"
#include "EgGeometries.h"
#include "eg_basics.h"
#include "sokol/sokol_fetch.h"
#include "libs/stb/stb_image.h"
#include "sokol_source.h"


static void fetch_callback(const sfetch_response_t* response)
{
	EgWorldEntity * userdata = response->user_data;
	ecs_world_t * world = userdata->world;
	ecs_entity_t entity = userdata->entity;
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
		ecs_set(world, entity, EgRectangleI32, {png_width, png_height});
		ecs_set(world, entity, EgImage, {pixels, num_channels});
		ecs_add_pair(world, entity, EgState, EgValid);
	}
}

static uint8_t file_buffer[256 * 1024];
static void System_Update(ecs_iter_t *it)
{
	EG_ITER_INFO(it);
	EgPath *path = ecs_term(it, EgPath, 1);
	//EgImage *image = ecs_term(it, EgImage, 2);
	//ecs_term(it, EgUpdate, 3); Tag
	EgWorldEntity userdata;
	userdata.world = it->world;
	for (int i = 0; i < it->count; i ++)
	{
		char const * p = path[i].value;
		ecs_add_pair(it->world, it->entities[i], EgState, EgUpdating);
		//ecs_remove(it->world, it->entities[i], EgUpdate);
		//ecs_add(it->world, it->entities[i], EgUpdating);
		userdata.entity = it->entities[i];
		sfetch_send(&(sfetch_request_t){
		.path = p,
		.callback = fetch_callback,
		.buffer_ptr = file_buffer,
		.buffer_size = sizeof(file_buffer),
		.user_data_ptr = &userdata,
		.user_data_size = sizeof(EgWorldEntity)
		});
	}
}

// bool sg_uninit_image(sg_image img_id)
// void sg_update_image(sg_image img_id, const sg_image_data* data)

static void System_Texture(ecs_iter_t *it)
{
	EgImage *image = ecs_term(it, EgImage, 1); // parent
	EgRectangleI32 *r = ecs_term(it, EgRectangleI32, 2); // parent
	EgTexture *tex = ecs_term(it, EgTexture, 3);
	for (int i = 0; i < it->count; i ++)
	{
		void * buffer = image[0].buffer;
		int width = r[0].width;
		int height = r[0].height;
		int pixel_format = tex[i].pixel_format;
		int min_filter = tex[i].min_filter;
		int mag_filter = tex[i].mag_filter;
		sg_image a = {tex[i].slot};
		sg_init_image(a, &(sg_image_desc){
		.width = width,
		.height = height,
		.pixel_format = pixel_format,
		.min_filter = min_filter,
		.mag_filter = mag_filter,
		.data.subimage[0][0] =
		{
		.ptr = buffer,
		.size = (size_t)(width * height * 4),
		}
		});
		//stbi_image_free(pixels);
		ecs_set_ptr(it->world, it->entities[i], EgRectangleI32, r);
		ecs_add_pair(it->world, it->entities[i], EgState, EgValid);
	}
}

void EgSokolFetchImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSokolFetch);
	ECS_IMPORT(world, EgResources);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgPath, EgImage, (eg.resources.State, eg.resources.Update)",
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Update
	});

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgImage(parent), EgRectangleI32(parent), EgTexture, (eg.resources.State, eg.resources.Update)",
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Texture
	});


	/*
	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.terms = {
	{ .id = ecs_id(EgPath), .inout = EcsIn},
	{ .id = ecs_id(EgImage), .inout = EcsOut},
	{ .id = ecs_id(EgUpdate)}
	},
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Update
	});

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.terms = {
	{ .id = ecs_id(EgTexture), .inout = EcsOut},
	{ .id = ecs_id(EgImage), .inout = EcsIn},
	{ .id = ecs_id(EgRectangleI32), .inout = EcsIn},
	{ .id = ecs_id(EgValid)}
	},
	.entity.add = {ecs_dependson(EcsOnUpdate)},
	.callback = System_Texture
	});
	*/
}
