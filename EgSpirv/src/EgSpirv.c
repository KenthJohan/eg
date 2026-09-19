#include "EgSpirv.h"
#include <spirv_cross/spirv_cross_c.h>
#include <stdio.h>
#include <stdlib.h>

ECS_COMPONENT_DECLARE(EgSpirvShaderCreateInfo);
ECS_COMPONENT_DECLARE(EgSpirvShader);
ECS_COMPONENT_DECLARE(EgSpirvShaderInput);

static void EgSpirvShader_free(EgSpirvShader *shader)
{
	if (shader->words) {
		ecs_os_free(shader->words);
		shader->words = NULL;
	}
	shader->word_count = 0;
}

static bool EgSpirv_load(const char *path, uint32_t **words_out, uint32_t *count_out)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		return false;
	}
	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return false;
	}
	long size = ftell(file);
	if (size <= 0 || (size % (long)sizeof(uint32_t)) != 0) {
		fclose(file);
		return false;
	}
	rewind(file);

	uint32_t *words = ecs_os_malloc((size_t)size);
	if (!words || fread(words, 1, (size_t)size, file) != (size_t)size) {
		ecs_os_free(words);
		fclose(file);
		return false;
	}
	fclose(file);

	*words_out = words;
	*count_out = (uint32_t)(size / (long)sizeof(uint32_t));
	return true;
}

static bool EgSpirv_reflect_inputs(
	ecs_world_t *world,
	ecs_entity_t shader_entity,
	const EgSpirvShaderCreateInfo *create,
	const EgSpirvShader *shader)
{
	spvc_context context = NULL;
	spvc_parsed_ir ir = NULL;
	spvc_compiler compiler = NULL;
	spvc_resources resources = NULL;
	const spvc_reflected_resource *inputs = NULL;
	size_t input_count = 0;

	if (spvc_context_create(&context) != SPVC_SUCCESS ||
		spvc_context_parse_spirv(context, shader->words, shader->word_count, &ir) != SPVC_SUCCESS ||
		spvc_context_create_compiler(context, SPVC_BACKEND_NONE, ir,
			SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler) != SPVC_SUCCESS ||
		spvc_compiler_get_execution_model(compiler) != (SpvExecutionModel)create->stage ||
		spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS ||
		spvc_resources_get_resource_list_for_type(resources,
			SPVC_RESOURCE_TYPE_STAGE_INPUT, &inputs, &input_count) != SPVC_SUCCESS) {
		if (context) {
			spvc_context_destroy(context);
		}
		return false;
	}

	for (size_t i = 0; i < input_count; i++) {
		const spvc_reflected_resource *input = &inputs[i];
		spvc_type type = spvc_compiler_get_type_handle(compiler, input->type_id);
		ecs_entity_t child = ecs_entity(world, {
			.name = input->name ? input->name : "input"
		});
		ecs_add_pair(world, child, EcsChildOf, shader_entity);
		ecs_set(world, child, EgGpusLocation, {
			.location = (int32_t)spvc_compiler_get_decoration(
				compiler, input->id, SpvDecorationLocation)
		});
		ecs_set(world, child, EgSpirvShaderInput, {
			.base_type = (int32_t)spvc_type_get_basetype(type),
			.vector_size = spvc_type_get_vector_size(type),
			.bit_width = spvc_type_get_bit_width(type)
		});
	}

	spvc_context_destroy(context);
	return true;
}

static void EgSpirvShader_create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgSpirvShaderCreateInfo *create = ecs_field(it, EgSpirvShaderCreateInfo, 0);

	for (int i = 0; i < it->count; i++) {
		ecs_entity_t entity = it->entities[i];
		uint32_t *words = NULL;
		uint32_t word_count = 0;
		if (!EgSpirv_load(create[i].path, &words, &word_count)) {
			ecs_err("Failed to load SPIR-V shader: %s", create[i].path);
			ecs_enable(world, entity, false);
			continue;
		}

		EgSpirvShader shader = {.words = words, .word_count = word_count};
		if (!EgSpirv_reflect_inputs(world, entity, &create[i], &shader)) {
			ecs_err("Failed to reflect SPIR-V shader: %s", create[i].path);
			EgSpirvShader_free(&shader);
			ecs_enable(world, entity, false);
			continue;
		}
		ecs_set_ptr(world, entity, EgSpirvShader, &shader);
	}
}

static void EgSpirvShader_remove(ecs_iter_t *it)
{
	EgSpirvShader *shader = ecs_field(it, EgSpirvShader, 0);
	for (int i = 0; i < it->count; i++) {
		EgSpirvShader_free(&shader[i]);
	}
}

void EgSpirvImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSpirv);
	ecs_set_name_prefix(world, "EgSpirv");
	ECS_IMPORT(world, EgGpus);

	ECS_COMPONENT_DEFINE(world, EgSpirvShaderCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgSpirvShader);
	ECS_COMPONENT_DEFINE(world, EgSpirvShaderInput);

	ecs_struct(world, {.entity = ecs_id(EgSpirvShaderCreateInfo), .members = {
		{.name = "path", .type = ecs_id(ecs_string_t)},
		{.name = "stage", .type = ecs_id(ecs_i32_t)}
	}});
	ecs_struct(world, {.entity = ecs_id(EgSpirvShader), .members = {
		{.name = "words", .type = ecs_id(ecs_uptr_t)},
		{.name = "word_count", .type = ecs_id(ecs_u32_t)}
	}});
	ecs_struct(world, {.entity = ecs_id(EgSpirvShaderInput), .members = {
		{.name = "base_type", .type = ecs_id(ecs_i32_t)},
		{.name = "vector_size", .type = ecs_id(ecs_u32_t)},
		{.name = "bit_width", .type = ecs_id(ecs_u32_t)}
	}});

	ecs_set_hooks(world, EgSpirvShader, {.on_remove = EgSpirvShader_remove});
	ecs_system_init(world, &(ecs_system_desc_t){
		.entity = ecs_entity(world, {.name = "EgSpirvShader_Create"}),
		.callback = EgSpirvShader_create,
		.phase = EcsOnUpdate,
		.query.terms = {
			{.id = ecs_id(EgSpirvShaderCreateInfo), .src.id = EcsSelf},
			{.id = ecs_id(EgSpirvShader), .oper = EcsNot}
		}
	});
}
