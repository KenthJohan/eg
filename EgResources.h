#pragma once
#include "flecs.h"

typedef struct
{
	ecs_f32_t x;
	ecs_f32_t y;
	void * buffer;
} EgFile;

typedef struct
{
	uint32_t fetched_offset;
	uint32_t fetched_size;
	void* buffer_ptr;
	uint32_t buffer_size;
} EgChunk;

typedef struct
{
	void * buffer;
	int num_channels;
} EgImage;

/*

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

*/
typedef struct
{
	int slot;
	int pixel_format;
	int min_filter;
	int mag_filter;
} EgTexture;

typedef struct
{
	ecs_string_t value;
} EgPath;

typedef struct
{
	ecs_world_t * world;
	ecs_entity_t entity;
} EgWorldEntity;




extern ECS_DECLARE(EgState);
extern ECS_DECLARE(EgUpdating);
extern ECS_DECLARE(EgUpdate);
extern ECS_DECLARE(EgValid);
extern ECS_DECLARE(EgMoving);
extern ECS_DECLARE(EgError);

extern ECS_COMPONENT_DECLARE(EgImage);
extern ECS_COMPONENT_DECLARE(EgPath);
extern ECS_COMPONENT_DECLARE(EgChunk);
extern ECS_COMPONENT_DECLARE(EgTexture);


void EgResourcesImport(ecs_world_t *world);
