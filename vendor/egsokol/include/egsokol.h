#pragma once

#include <flecs.h>
#include <egwindows.h>
#include <sokol_gfx.h>
#include <sokol_debugtext.h>
#include <sokol_app.h>

typedef enum {
	SgPrimitiveTypeDEFAULT, // value 0 reserved for default-init
	SgPrimitiveTypePOINTS,
	SgPrimitiveTypeLINES,
	SgPrimitiveTypeLINE_STRIP,
	SgPrimitiveTypeTRIANGLES,
	SgPrimitiveTypeTRIANGLE_STRIP,
	SgPrimitiveTypeNUM,
	SgPrimitiveTypeFORCE_U32 = 0x7FFFFFFF
} SgPrimitiveType;

typedef enum {
	SgCullModeDEFAULT, // value 0 reserved for default-init
	SgCullModeNONE,
	SgCullModeFRONT,
	SgCullModeBACK,
	SgCullModeNUM,
	SgCullModeFORCE_U32 = 0x7FFFFFFF
} SgCullMode;

typedef enum {
	SgIndexTypeDEFAULT, // value 0 reserved for default-init
	SgIndexTypeNONE,
	SgIndexTypeUINT16,
	SgIndexTypeUINT32,
	SgIndexTypeNUM,
	SgIndexTypeFORCE_U32 = 0x7FFFFFFF
} SgIndexType;

typedef enum {
	SgVertexFormatINVALID,
	SgVertexFormatFLOAT,
	SgVertexFormatFLOAT2,
	SgVertexFormatFLOAT3,
	SgVertexFormatFLOAT4,
	SgVertexFormatBYTE4,
	SgVertexFormatBYTE4N,
	SgVertexFormatUBYTE4,
	SgVertexFormatUBYTE4N,
	SgVertexFormatSHORT2,
	SgVertexFormatSHORT2N,
	SgVertexFormatUSHORT2N,
	SgVertexFormatSHORT4,
	SgVertexFormatSHORT4N,
	SgVertexFormatUSHORT4N,
	SgVertexFormatUINT10_N2,
	SgVertexFormatHALF2,
	SgVertexFormatHALF4,
	SgVertexFormatNUM,
	SgVertexFormatFORCE_U32 = 0x7FFFFFFF
} SgVertexFormat;

typedef enum {
	SgUniformTypeINVALID,
	SgUniformTypeFLOAT,
	SgUniformTypeFLOAT2,
	SgUniformTypeFLOAT3,
	SgUniformTypeFLOAT4,
	SgUniformTypeINT,
	SgUniformTypeINT2,
	SgUniformTypeINT3,
	SgUniformTypeINT4,
	SgUniformTypeMAT4,
	SgUniformTypeNUM,
	SgUniformTypeFORCE_U32 = 0x7FFFFFFF
} SgUniformType;

typedef struct
{
	ecs_entity_t shader;
	SgPrimitiveType primtype;
	SgCullMode cullmode;
	SgIndexType indextype;
} SgPipelineCreate;

typedef struct
{
	uint32_t id;
} SgPipeline;

typedef struct
{
	ecs_string_t filename_vs;
	ecs_string_t filename_fs;
	ecs_entity_t ubs;
	ecs_entity_t attrs;
} SgShaderCreate;

typedef struct
{
	uint32_t id;
} SgShader;

typedef struct
{
	ecs_i32_t index;
	SgVertexFormat format;
	ecs_i32_t offset;
	ecs_i32_t buffer_index;
} SgLocation;


typedef struct
{
	ecs_i32_t stride;
	ecs_i32_t step_func;
	ecs_i32_t step_rate;
} SgVertexBufferLayout;

typedef struct
{
	ecs_i32_t dummy;
} SgAttributes;

typedef struct
{
	ecs_i32_t dummy;
} SgUniformBlocks;

typedef struct
{
	ecs_i32_t index;
	ecs_i32_t size;
} SgUniformBlock;

typedef struct
{
	ecs_i32_t index;
	ecs_i32_t array_count;
	SgUniformType type;
} SgUniform;

extern ECS_COMPONENT_DECLARE(SgPipelineCreate);
extern ECS_COMPONENT_DECLARE(SgPipeline);
extern ECS_COMPONENT_DECLARE(SgShaderCreate);
extern ECS_COMPONENT_DECLARE(SgShader);
extern ECS_COMPONENT_DECLARE(SgVertexBufferLayout);
extern ECS_COMPONENT_DECLARE(SgLocation);
extern ECS_COMPONENT_DECLARE(SgAttributes);
extern ECS_COMPONENT_DECLARE(SgUniformBlocks);
extern ECS_COMPONENT_DECLARE(SgVertexFormat);
extern ECS_COMPONENT_DECLARE(SgUniformType);
extern ECS_COMPONENT_DECLARE(SgIndexType);
extern ECS_COMPONENT_DECLARE(SgPrimitiveType);
extern ECS_COMPONENT_DECLARE(SgCullMode);
extern ECS_COMPONENT_DECLARE(SgUniformBlock);
extern ECS_COMPONENT_DECLARE(SgUniform);


void SgImport(ecs_world_t *world);

void egsokol_flecs_event_cb(const sapp_event *evt, Window *window);