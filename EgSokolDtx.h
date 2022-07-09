#pragma once
#include "flecs.h"

/*
	sdtx_setup(&(sdtx_desc_t){
	.context_pool_size = 1,
	.fonts[0] = sdtx_font_oric()
	});

typedef struct sdtx_desc_t {
	int context_pool_size;                  // max number of rendering contexts that can be created, default: 8
	int printf_buf_size;                    // size of internal buffer for snprintf(), default: 4096
	sdtx_font_desc_t fonts[SDTX_MAX_FONTS]; // up to 8 fonts descriptions
	sdtx_context_desc_t context;            // the default context creation parameters
	sdtx_allocator_t allocator;             // optional memory allocation overrides (default: malloc/free)
} sdtx_desc_t;
*/
typedef struct
{
	ecs_i32_t context_pool_size;
} EgSokolDtxConfig;

extern ECS_COMPONENT_DECLARE(EgSokolDtxConfig);

extern ECS_DECLARE(EgSokolDtxDraw);


void EgSokolDtxImport(ecs_world_t *world);
