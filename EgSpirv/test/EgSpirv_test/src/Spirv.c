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
	test_assert(ecs_id(EgSpirvReflect) != 0);
	test_assert(ecs_id(EgSpirvShaderInput) != 0);
	test_assert(EgSpirvBaseType != 0);
	test_assert(ecs_lookup_child(world, EgSpirvBaseType, "FP32") != 0);
}

static EgFsContent Spirv_load_test_shader(const char *path)
{
	FILE *file = fopen(path, "rb");
	test_assert(file != NULL);
	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		test_assert(false);
	}
	long size = ftell(file);
	if (size <= 0 || (size % (long)sizeof(uint32_t)) != 0) {
		fclose(file);
		test_assert(false);
	}
	rewind(file);

	uint32_t *words = ecs_os_malloc((size_t)size);
	test_assert(words != NULL);
	test_assert(fread(words, 1, (size_t)size, file) == (size_t)size);
	fclose(file);

	return (EgFsContent){.data = words, .size = (uint32_t)(size / (long)sizeof(uint32_t)) * sizeof(uint32_t)};
}

void Spirv_test_reflect_vertex_inputs(void)
{
	ecs_entity_t shader = ecs_new(world);
	EgFsContent content = Spirv_load_test_shader("data/vertex.spv");
	ecs_set(world, shader, EgSpirvReflect, {.stage = SpvExecutionModelVertex});
	ecs_set_ptr(world, shader, EgFsContent, &content);
	ecs_progress(world, 0.0f);

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

void Spirv_test_invalid_shader_data_disables_entity(void)
{
	ecs_entity_t shader = ecs_new(world);
	EgFsContent content = {.data = NULL, .size = 0};
	ecs_set(world, shader, EgSpirvReflect, {.stage = SpvExecutionModelVertex});
	ecs_set_ptr(world, shader, EgFsContent, &content);
	ecs_progress(world, 0.0f);

	test_assert(ecs_has_id(world, shader, EcsDisabled));
}

void Spirv_test_wrong_stage_disables_entity(void)
{
	ecs_entity_t shader = ecs_new(world);
	EgFsContent content = Spirv_load_test_shader("data/vertex.spv");
	ecs_set(world, shader, EgSpirvReflect, {.stage = SpvExecutionModelFragment});
	ecs_set_ptr(world, shader, EgFsContent, &content);
	ecs_progress(world, 0.0f);

	test_assert(ecs_has_id(world, shader, EcsDisabled));
}
