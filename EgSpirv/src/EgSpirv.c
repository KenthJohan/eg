#include "EgSpirv.h"
#include <ecsx.h>
#include <spirv_cross/spirv_cross_c.h>
#include <stdio.h>
#include <stdlib.h>
#include "EgSpirvReflect.h"

ECS_COMPONENT_DECLARE(EgSpirvReflect);
ECS_COMPONENT_DECLARE(EgSpirvShaderInput);
ecs_entity_t EgSpirvBaseType;

void EgSpirvImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSpirv);
	ecs_set_name_prefix(world, "EgSpirv");
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgFs);

	ECS_COMPONENT_DEFINE(world, EgSpirvShaderInput);
	ECS_COMPONENT_DEFINE(world, EgSpirvReflect);

	EgSpirvBaseType = ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_entity(world, {.name = "BaseType"}),
	.constants = {
	{.name = "Unknown", .value = SPVC_BASETYPE_UNKNOWN},
	{.name = "Void", .value = SPVC_BASETYPE_VOID},
	{.name = "Boolean", .value = SPVC_BASETYPE_BOOLEAN},
	{.name = "Int8", .value = SPVC_BASETYPE_INT8},
	{.name = "UInt8", .value = SPVC_BASETYPE_UINT8},
	{.name = "Int16", .value = SPVC_BASETYPE_INT16},
	{.name = "UInt16", .value = SPVC_BASETYPE_UINT16},
	{.name = "Int32", .value = SPVC_BASETYPE_INT32},
	{.name = "UInt32", .value = SPVC_BASETYPE_UINT32},
	{.name = "Int64", .value = SPVC_BASETYPE_INT64},
	{.name = "UInt64", .value = SPVC_BASETYPE_UINT64},
	{.name = "AtomicCounter", .value = SPVC_BASETYPE_ATOMIC_COUNTER},
	{.name = "FP16", .value = SPVC_BASETYPE_FP16},
	{.name = "FP32", .value = SPVC_BASETYPE_FP32},
	{.name = "FP64", .value = SPVC_BASETYPE_FP64},
	{.name = "Struct", .value = SPVC_BASETYPE_STRUCT},
	{.name = "Image", .value = SPVC_BASETYPE_IMAGE},
	{.name = "SampledImage", .value = SPVC_BASETYPE_SAMPLED_IMAGE},
	{.name = "Sampler", .value = SPVC_BASETYPE_SAMPLER},
	{.name = "AccelerationStructure", .value = SPVC_BASETYPE_ACCELERATION_STRUCTURE}}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgSpirvReflect),
	.members = {
	{.name = "stage", .type = ecs_id(EgGpusShaderStage)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgSpirvShaderInput),
	.members = {
	{.name = "base_type", .type = EgSpirvBaseType},
	{.name = "type", .type = ecs_id(ecs_entity_t)},
	{.name = "vector_size", .type = ecs_id(ecs_u32_t)},
	{.name = "bit_width", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgSpirvReflect_System"}),
	.callback    = EgSpirvReflect_System,
	.phase       = EcsOnUpdate,
	.query.terms = {
	{.id = ecs_id(EgSpirvReflect), .src.id = EcsSelf}, // removes this
	{.id = ecs_id(EgFsContent), .trav = EcsDependsOn, .src.id = EcsUp},
	}});
}
