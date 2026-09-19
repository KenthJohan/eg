#include <EgSpirv_test.h>
#include <EgSpirv.h>
#include <spirv_cross/spirv_cross_c.h>

static ecs_world_t *world;

void Spirv_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgSpirv);
}

void Spirv_teardown(void)
{
	ecs_fini(world);
}

void Spirv_test_import(void)
{
	test_assert(ecs_id(EgSpirvShaderCreateInfo) != 0);
	test_assert(ecs_id(EgSpirvShader) != 0);
	test_assert(ecs_id(EgSpirvShaderInput) != 0);
	test_assert(EgSpirvBaseType != 0);
	test_assert(ecs_lookup_child(world, EgSpirvBaseType, "FP32") != 0);
}

void Spirv_test_reflect_vertex_inputs(void)
{
	ecs_entity_t shader = ecs_new(world);
	ecs_set(world, shader, EgSpirvShaderCreateInfo, {
		.path = "data/vertex.spv",
		.stage = SpvExecutionModelVertex
	});
	ecs_progress(world, 0.0f);

	const EgSpirvShader *program = ecs_get(world, shader, EgSpirvShader);
	test_assert(program != NULL);
	test_assert(program->words != NULL);
	test_assert(program->word_count > 0);

	ecs_entity_t a_pos = ecs_lookup_child(world, shader, "aPos");
	ecs_entity_t a_uv = ecs_lookup_child(world, shader, "aUV");
	ecs_entity_t a_color = ecs_lookup_child(world, shader, "aColor");
	test_assert(a_pos != 0);
	test_assert(a_uv != 0);
	test_assert(a_color != 0);

	const EgGpusLocation *pos_location = ecs_get(world, a_pos, EgGpusLocation);
	const EgGpusLocation *uv_location = ecs_get(world, a_uv, EgGpusLocation);
	const EgGpusLocation *color_location = ecs_get(world, a_color, EgGpusLocation);
	test_assert(pos_location && pos_location->location == 0);
	test_assert(uv_location && uv_location->location == 1);
	test_assert(color_location && color_location->location == 2);

	const EgSpirvShaderInput *pos = ecs_get(world, a_pos, EgSpirvShaderInput);
	const EgSpirvShaderInput *uv = ecs_get(world, a_uv, EgSpirvShaderInput);
	const EgSpirvShaderInput *color = ecs_get(world, a_color, EgSpirvShaderInput);
	test_assert(pos && pos->base_type == SPVC_BASETYPE_FP32 && pos->type == ecs_id(ecs_f32_t) && pos->vector_size == 2 && pos->bit_width == 32);
	test_assert(uv && uv->base_type == SPVC_BASETYPE_FP32 && uv->type == ecs_id(ecs_f32_t) && uv->vector_size == 2 && uv->bit_width == 32);
	test_assert(color && color->base_type == SPVC_BASETYPE_FP32 && color->type == ecs_id(ecs_f32_t) && color->vector_size == 4 && color->bit_width == 32);
}

void Spirv_test_invalid_shader_path_disables_entity(void)
{
	ecs_entity_t shader = ecs_new(world);
	ecs_set(world, shader, EgSpirvShaderCreateInfo, {
		.path = "missing.spv",
		.stage = SpvExecutionModelVertex
	});
	ecs_progress(world, 0.0f);

	test_assert(ecs_has_id(world, shader, EcsDisabled));
	test_assert(!ecs_has(world, shader, EgSpirvShader));
}

void Spirv_test_wrong_stage_disables_entity(void)
{
	ecs_entity_t shader = ecs_new(world);
	ecs_set(world, shader, EgSpirvShaderCreateInfo, {
		.path = "data/vertex.spv",
		.stage = SpvExecutionModelFragment
	});
	ecs_progress(world, 0.0f);

	test_assert(ecs_has_id(world, shader, EcsDisabled));
	test_assert(!ecs_has(world, shader, EgSpirvShader));
}
