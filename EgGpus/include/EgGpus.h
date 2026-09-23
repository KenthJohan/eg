#pragma once
#include <flecs.h>

#define EG_GPUS_LOGTAG "[EgGpus] "

typedef struct
{
	void *object;
} EgGpusDevice;

typedef struct
{
	bool debug;
} EgGpusDeviceCreateInfo;

typedef struct
{
	void *object;
} EgGpusTexture;

typedef enum {
	EgGpusTextureUsageDepthStencil,
	EgGpusTextureUsageSampled,
} EgGpusTextureUsage;

typedef enum {
	EgGpusTextureFormatD16Unorm,
	EgGpusTextureFormatRgba8Unorm,
} EgGpusTextureFormat;

typedef struct
{
	uint32_t sample_count;
	EgGpusTextureUsage usage;
	EgGpusTextureFormat format;
} EgGpusTextureCreateInfo;

typedef struct
{
	void *object;
} EgGpusBuffer;

typedef enum {
	EgGpusBufferUsageVertex,
	EgGpusBufferUsageIndex,
	EgGpusBufferUsageUniform,
} EgGpusBufferUsage;

typedef struct
{
	uint32_t size;
	EgGpusBufferUsage usage;
} EgGpusBufferCreateInfo;

typedef struct
{
	void *object;
} EgGpusSampler;

typedef enum {
	EgGpusSamplerFilterNearest,
	EgGpusSamplerFilterLinear,
} EgGpusSamplerFilter;

typedef enum {
	EgGpusSamplerAddressModeClampToEdge,
	EgGpusSamplerAddressModeRepeat,
} EgGpusSamplerAddressMode;

typedef struct
{
	EgGpusSamplerFilter      min_filter;
	EgGpusSamplerFilter      mag_filter;
	EgGpusSamplerAddressMode address_mode_u;
	EgGpusSamplerAddressMode address_mode_v;
	EgGpusSamplerAddressMode address_mode_w;
} EgGpusSamplerCreateInfo;

typedef struct
{
	int32_t location;
} EgGpusLocation;

typedef struct
{
	void    *object;
	uint32_t info_num_vertex_attributes;
} EgGpusGraphicsPipeline;

typedef struct
{
	int32_t      sample_count;
} EgGpusGraphicsPipelineCreateInfo;

typedef struct
{
	void *object;
} EgGpusShaderVertex;

typedef struct
{
	void *object;
} EgGpusShaderFragment;

typedef enum {
	EgGpusShaderStageVertex,
	EgGpusShaderStageFragment,
} EgGpusShaderStage;

typedef struct
{
	EgGpusShaderStage stage;
} EgGpusShaderCreateInfo;

extern ECS_COMPONENT_DECLARE(EgGpusDevice);
extern ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusTexture);
extern ECS_COMPONENT_DECLARE(EgGpusTextureUsage);
extern ECS_COMPONENT_DECLARE(EgGpusTextureFormat);
extern ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusBuffer);
extern ECS_COMPONENT_DECLARE(EgGpusBufferUsage);
extern ECS_COMPONENT_DECLARE(EgGpusBufferCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusSampler);
extern ECS_COMPONENT_DECLARE(EgGpusSamplerFilter);
extern ECS_COMPONENT_DECLARE(EgGpusSamplerAddressMode);
extern ECS_COMPONENT_DECLARE(EgGpusSamplerCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusLocation);
extern ECS_COMPONENT_DECLARE(EgGpusGraphicsPipeline);
extern ECS_COMPONENT_DECLARE(EgGpusGraphicsPipelineCreateInfo);
extern ECS_COMPONENT_DECLARE(EgGpusShaderVertex);
extern ECS_COMPONENT_DECLARE(EgGpusShaderFragment);
extern ECS_COMPONENT_DECLARE(EgGpusShaderStage);
extern ECS_COMPONENT_DECLARE(EgGpusShaderCreateInfo);

void EgGpusImport(ecs_world_t *world);
