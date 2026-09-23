#include "EgGpus.h"

ECS_COMPONENT_DECLARE(EgGpusDevice);
ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusTexture);
ECS_COMPONENT_DECLARE(EgGpusTextureUsage);
ECS_COMPONENT_DECLARE(EgGpusTextureFormat);
ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusBuffer);
ECS_COMPONENT_DECLARE(EgGpusBufferUsage);
ECS_COMPONENT_DECLARE(EgGpusBufferCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusSampler);
ECS_COMPONENT_DECLARE(EgGpusSamplerFilter);
ECS_COMPONENT_DECLARE(EgGpusSamplerAddressMode);
ECS_COMPONENT_DECLARE(EgGpusSamplerCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusLocation);
ECS_COMPONENT_DECLARE(EgGpusGraphicsPipeline);
ECS_COMPONENT_DECLARE(EgGpusGraphicsPipelineCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusShaderVertex);
ECS_COMPONENT_DECLARE(EgGpusShaderFragment);
ECS_COMPONENT_DECLARE(EgGpusShaderStage);
ECS_COMPONENT_DECLARE(EgGpusShaderCreateInfo);

void EgGpusImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGpus);
	ecs_set_name_prefix(world, "EgGpus");

	ECS_COMPONENT_DEFINE(world, EgGpusDevice);
	ECS_COMPONENT_DEFINE(world, EgGpusDeviceCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusTexture);
	ECS_COMPONENT_DEFINE(world, EgGpusTextureUsage);
	ECS_COMPONENT_DEFINE(world, EgGpusTextureFormat);
	ECS_COMPONENT_DEFINE(world, EgGpusTextureCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusBuffer);
	ECS_COMPONENT_DEFINE(world, EgGpusBufferUsage);
	ECS_COMPONENT_DEFINE(world, EgGpusBufferCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusSampler);
	ECS_COMPONENT_DEFINE(world, EgGpusSamplerFilter);
	ECS_COMPONENT_DEFINE(world, EgGpusSamplerAddressMode);
	ECS_COMPONENT_DEFINE(world, EgGpusSamplerCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusLocation);
	ECS_COMPONENT_DEFINE(world, EgGpusGraphicsPipeline);
	ECS_COMPONENT_DEFINE(world, EgGpusGraphicsPipelineCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusShaderVertex);
	ECS_COMPONENT_DEFINE(world, EgGpusShaderFragment);
	ECS_COMPONENT_DEFINE(world, EgGpusShaderStage);
	ECS_COMPONENT_DEFINE(world, EgGpusShaderCreateInfo);

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgGpusShaderStage),
	.constants = {
	{.name = "Vertex", .value = EgGpusShaderStageVertex},
	{.name = "Fragment", .value = EgGpusShaderStageFragment},
	}});

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgGpusTextureUsage),
	.constants = {
	{.name = "DepthStencil", .value = EgGpusTextureUsageDepthStencil},
	{.name = "Sampled", .value = EgGpusTextureUsageSampled},
	}});

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgGpusTextureFormat),
	.constants = {
	{.name = "D16Unorm", .value = EgGpusTextureFormatD16Unorm},
	{.name = "Rgba8Unorm", .value = EgGpusTextureFormatRgba8Unorm},
	}});

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgGpusBufferUsage),
	.constants = {
	{.name = "Vertex", .value = EgGpusBufferUsageVertex},
	{.name = "Index", .value = EgGpusBufferUsageIndex},
	{.name = "Uniform", .value = EgGpusBufferUsageUniform},
	}});

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgGpusSamplerFilter),
	.constants = {
	{.name = "Nearest", .value = EgGpusSamplerFilterNearest},
	{.name = "Linear", .value = EgGpusSamplerFilterLinear},
	}});

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgGpusSamplerAddressMode),
	.constants = {
	{.name = "ClampToEdge", .value = EgGpusSamplerAddressModeClampToEdge},
	{.name = "Repeat", .value = EgGpusSamplerAddressModeRepeat},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusDeviceCreateInfo),
	.members = {
	{.name = "debug", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusDevice),
	.members = {
	{.name = "device", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusTexture),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusTextureCreateInfo),
	.members = {
	{.name = "sample_count", .type = ecs_id(ecs_u32_t)},
	{.name = "usage", .type = ecs_id(EgGpusTextureUsage)},
	{.name = "format", .type = ecs_id(EgGpusTextureFormat)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusBuffer),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusBufferCreateInfo),
	.members = {
	{.name = "size", .type = ecs_id(ecs_u32_t)},
	{.name = "usage", .type = ecs_id(EgGpusBufferUsage)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusSampler),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusSamplerCreateInfo),
	.members = {
	{.name = "min_filter", .type = ecs_id(EgGpusSamplerFilter)},
	{.name = "mag_filter", .type = ecs_id(EgGpusSamplerFilter)},
	{.name = "address_mode_u", .type = ecs_id(EgGpusSamplerAddressMode)},
	{.name = "address_mode_v", .type = ecs_id(EgGpusSamplerAddressMode)},
	{.name = "address_mode_w", .type = ecs_id(EgGpusSamplerAddressMode)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusLocation),
	.members = {
	{.name = "location", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusGraphicsPipeline),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	{.name = "info_num_vertex_attributes", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusGraphicsPipelineCreateInfo),
	.members = {
	{.name = "sample_count", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusShaderVertex),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusShaderFragment),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusShaderCreateInfo),
	.members = {
	{.name = "stage", .type = ecs_id(EgGpusShaderStage)},
	}});
}
