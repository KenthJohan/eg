#include "EgSpirvReflect.h"
#include "EgSpirv.h"
#include <EgFs.h>
#include <ecsx.h>
#include <spirv_cross/spirv_cross_c.h>

static ecs_entity_t EgSpirv_flecs_type(spvc_basetype base_type)
{
	switch (base_type) {
	case SPVC_BASETYPE_BOOLEAN:
		return ecs_id(ecs_bool_t);
	case SPVC_BASETYPE_INT8:
		return ecs_id(ecs_i8_t);
	case SPVC_BASETYPE_UINT8:
		return ecs_id(ecs_u8_t);
	case SPVC_BASETYPE_INT16:
		return ecs_id(ecs_i16_t);
	case SPVC_BASETYPE_UINT16:
		return ecs_id(ecs_u16_t);
	case SPVC_BASETYPE_INT32:
		return ecs_id(ecs_i32_t);
	case SPVC_BASETYPE_UINT32:
		return ecs_id(ecs_u32_t);
	case SPVC_BASETYPE_INT64:
		return ecs_id(ecs_i64_t);
	case SPVC_BASETYPE_UINT64:
		return ecs_id(ecs_u64_t);
	case SPVC_BASETYPE_FP32:
		return ecs_id(ecs_f32_t);
	case SPVC_BASETYPE_FP64:
		return ecs_id(ecs_f64_t);
	default:
		return 0;
	}
}

static SpvExecutionModel EgSpirv_execution_model(EgGpusShaderStage stage)
{
	switch (stage) {
	case EgGpusShaderStageVertex:
		return SpvExecutionModelVertex;
	case EgGpusShaderStageFragment:
		return SpvExecutionModelFragment;
	default:
		return SpvExecutionModelMax;
	}
}

static bool EgSpirv_reflect_inputs(ecs_world_t *world, ecs_entity_t shader_entity, const EgSpirvReflect *reflect, const EgFsContent *content)
{
	spvc_context                   context     = NULL;
	spvc_parsed_ir                 ir          = NULL;
	spvc_compiler                  compiler    = NULL;
	spvc_resources                 resources   = NULL;
	const spvc_reflected_resource *inputs      = NULL;
	size_t                         input_count = 0;

	if (!content || !content->data || content->size == 0 || (content->size % sizeof(uint32_t)) != 0) {
		return false;
	}

	uint32_t *words = (uint32_t *)content->data;
	uint32_t  count = (uint32_t)(content->size / sizeof(uint32_t));

	if (spvc_context_create(&context) != SPVC_SUCCESS ||
	    spvc_context_parse_spirv(context, words, count, &ir) != SPVC_SUCCESS ||
	    spvc_context_create_compiler(context, SPVC_BACKEND_NONE, ir,
	    SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler) != SPVC_SUCCESS ||
	    spvc_compiler_get_execution_model(compiler) != EgSpirv_execution_model(reflect->stage) ||
	    spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS ||
	    spvc_resources_get_resource_list_for_type(resources,
	    SPVC_RESOURCE_TYPE_STAGE_INPUT, &inputs, &input_count) != SPVC_SUCCESS) {
		if (context) {
			spvc_context_destroy(context);
		}
		return false;
	}

	for (size_t i = 0; i < input_count; i++) {
		const spvc_reflected_resource *input     = &inputs[i];
		spvc_type                      type      = spvc_compiler_get_type_handle(compiler, input->type_id);
		spvc_basetype                  base_type = spvc_type_get_basetype(type);
		ecs_entity_t                   child     = ecs_entity(world, {.name = input->name ? input->name : "input"});
		ecs_add_pair(world, child, EcsChildOf, shader_entity);
		ecs_set(world, child, EgGpusLocation, {.location = (int32_t)spvc_compiler_get_decoration(compiler, input->id, SpvDecorationLocation)});
		ecs_set(world, child, EgSpirvShaderInput, {.base_type = (int32_t)base_type, .type = EgSpirv_flecs_type(base_type), .vector_size = spvc_type_get_vector_size(type), .bit_width = spvc_type_get_bit_width(type)});
	}

	spvc_context_destroy(context);
	return true;
}

void EgSpirvReflect_System(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;

	EgSpirvReflect *r = ecs_field_self(it, EgSpirvReflect, 0);
	EgFsContent    *c = ecs_field_shared(it, EgFsContent, 1);

	for (int i = 0; i < it->count; ++i, ++r, ++c) {
		ecs_entity_t e = it->entities[i];
		ecs_remove(world, e, EgSpirvReflect);

		if (!EgSpirv_reflect_inputs(world, e, r, c)) {
			ecs_err("Failed to reflect SPIR-V shader for entity '%s'", ecs_get_name(world, e));
			ecs_enable(world, e, false);
		}
	}
}
