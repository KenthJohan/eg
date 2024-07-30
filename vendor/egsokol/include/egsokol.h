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
	SgPixelFormatDEFAULT,
	SgPixelFormatNONE,
	SgPixelFormatR8,
	SgPixelFormatR8SN,
	SgPixelFormatR8UI,
	SgPixelFormatR8SI,
	SgPixelFormatR16,
	SgPixelFormatR16SN,
	SgPixelFormatR16UI,
	SgPixelFormatR16SI,
	SgPixelFormatR16F,
	SgPixelFormatRG8,
	SgPixelFormatRG8SN,
	SgPixelFormatRG8UI,
	SgPixelFormatRG8SI,
	SgPixelFormatR32UI,
	SgPixelFormatR32SI,
	SgPixelFormatR32F,
	SgPixelFormatRG16,
	SgPixelFormatRG16SN,
	SgPixelFormatRG16UI,
	SgPixelFormatRG16SI,
	SgPixelFormatRG16F,
	SgPixelFormatRGBA8,
	SgPixelFormatSRGB8A8,
	SgPixelFormatRGBA8SN,
	SgPixelFormatRGBA8UI,
	SgPixelFormatRGBA8SI,
	SgPixelFormatBGRA8,
	SgPixelFormatRGB10A2,
	SgPixelFormatRG11B10F,
	SgPixelFormatRGB9E5,
	SgPixelFormatRG32UI,
	SgPixelFormatRG32SI,
	SgPixelFormatRG32F,
	SgPixelFormatRGBA16,
	SgPixelFormatRGBA16SN,
	SgPixelFormatRGBA16UI,
	SgPixelFormatRGBA16SI,
	SgPixelFormatRGBA16F,
	SgPixelFormatRGBA32UI,
	SgPixelFormatRGBA32SI,
	SgPixelFormatRGBA32F,
	SgPixelFormatDEPTH,
	SgPixelFormatDEPTH_STENCIL,
	SgPixelFormatBC1_RGBA,
	SgPixelFormatBC2_RGBA,
	SgPixelFormatBC3_RGBA,
	SgPixelFormatBC3_SRGBA,
	SgPixelFormatBC4_R,
	SgPixelFormatBC4_RSN,
	SgPixelFormatBC5_RG,
	SgPixelFormatBC5_RGSN,
	SgPixelFormatBC6H_RGBF,
	SgPixelFormatBC6H_RGBUF,
	SgPixelFormatBC7_RGBA,
	SgPixelFormatBC7_SRGBA,
	SgPixelFormatPVRTC_RGB_2BPP,      // FIXME: deprecated
	SgPixelFormatPVRTC_RGB_4BPP,      // FIXME: deprecated
	SgPixelFormatPVRTC_RGBA_2BPP,     // FIXME: deprecated
	SgPixelFormatPVRTC_RGBA_4BPP,     // FIXME: deprecated
	SgPixelFormatETC2_RGB8,
	SgPixelFormatETC2_SRGB8,
	SgPixelFormatETC2_RGB8A1,
	SgPixelFormatETC2_RGBA8,
	SgPixelFormatETC2_SRGB8A8,
	SgPixelFormatETC2_RG11,
	SgPixelFormatETC2_RG11SN,
	SgPixelFormatASTC_4x4_RGBA,
	SgPixelFormatASTC_4x4_SRGBA,
	SgPixelFormatNUM,
	SgPixelFormatFORCE_U32 = 0x7FFFFFFF
} SgPixelFormat;

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

typedef enum {
    EgSokol__SG_FILTER_DEFAULT, // value 0 reserved for default-init
    EgSokol_SG_FILTER_NONE,     // FIXME: deprecated
    EgSokol_SG_FILTER_NEAREST,
    EgSokol_SG_FILTER_LINEAR,
    EgSokol__SG_FILTER_NUM,
    EgSokol__SG_FILTER_FORCE_U32 = 0x7FFFFFFF
} SgFilter;



typedef struct
{
	ecs_i32_t stride;
	ecs_i32_t step_func;
	ecs_i32_t step_rate;
} SgVertexBufferLayout;


typedef struct
{
	SgFilter min_filter;
	SgFilter mag_filter;
} SgSamplerCreate;

typedef struct
{
	uint32_t id;
} SgSampler;

typedef struct
{
	ecs_i32_t width;
	ecs_i32_t height;
	ecs_i32_t slices;
	SgPixelFormat format;
} SgImageCreate;

typedef struct
{
	uint32_t id;
} SgImage;

typedef struct
{
	SgPrimitiveType primtype;
	SgCullMode cullmode;
	SgIndexType indextype;
	// TODO: Change to array when flecs explorer array gui is nice
	SgVertexBufferLayout buflayout0;
	SgVertexBufferLayout buflayout1;
	SgVertexBufferLayout buflayout2;
	SgVertexBufferLayout buflayout3;
	SgVertexBufferLayout buflayout4;
	SgVertexBufferLayout buflayout5;
	SgVertexBufferLayout buflayout6;
	SgVertexBufferLayout buflayout7;
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
	ecs_i32_t count;
} SgAttributes;

typedef struct
{
	ecs_i32_t count;
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


extern ECS_COMPONENT_DECLARE(SgFilter);
extern ECS_COMPONENT_DECLARE(SgSamplerCreate);
extern ECS_COMPONENT_DECLARE(SgSampler);
extern ECS_COMPONENT_DECLARE(SgImageCreate);
extern ECS_COMPONENT_DECLARE(SgImage);
extern ECS_COMPONENT_DECLARE(SgPipelineCreate);
extern ECS_COMPONENT_DECLARE(SgPipeline);
extern ECS_COMPONENT_DECLARE(SgShaderCreate);
extern ECS_COMPONENT_DECLARE(SgShader);
extern ECS_COMPONENT_DECLARE(SgVertexBufferLayout);
extern ECS_COMPONENT_DECLARE(SgLocation);
extern ECS_COMPONENT_DECLARE(SgAttributes);
extern ECS_COMPONENT_DECLARE(SgUniformBlocks);
extern ECS_COMPONENT_DECLARE(SgVertexFormat);
extern ECS_COMPONENT_DECLARE(SgPixelFormat);
extern ECS_COMPONENT_DECLARE(SgUniformType);
extern ECS_COMPONENT_DECLARE(SgIndexType);
extern ECS_COMPONENT_DECLARE(SgPrimitiveType);
extern ECS_COMPONENT_DECLARE(SgCullMode);
extern ECS_COMPONENT_DECLARE(SgUniformBlock);
extern ECS_COMPONENT_DECLARE(SgUniform);


void SgImport(ecs_world_t *world);

void egsokol_flecs_event_cb(const sapp_event *evt, Window *window);