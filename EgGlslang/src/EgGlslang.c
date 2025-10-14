/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477




Required libs: -lglslang -lSPIRV-Tools -lSPIRV-Tools-opt
"glslang-default-resource-limits",
"glslang",
"SPIRV-Tools-opt",
"SPIRV-Tools",
"stdc++",






*/

#include "EgGlslang.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>
#include <glslang/Include/glslang_c_interface.h>
// Required for use of glslang_default_resource
#include <glslang/Public/resource_limits_c.h>
#include <EgFs.h>

#include <ecsx.h>
#include <egmisc.h>

ECS_COMPONENT_DECLARE(EgGlslangCreate);
ECS_COMPONENT_DECLARE(EgGlslangProgram);

EgGlslangProgram compileShaderToSPIRV_Vulkan(glslang_stage_t stage, const char *shaderSource)
{
	printf("%s\n", shaderSource);
	const glslang_input_t input = {
	.language = GLSLANG_SOURCE_GLSL,
	.stage = stage,
	.client = GLSLANG_CLIENT_VULKAN,
	.client_version = GLSLANG_TARGET_VULKAN_1_2,
	.target_language = GLSLANG_TARGET_SPV,
	.target_language_version = GLSLANG_TARGET_SPV_1_5,
	.code = shaderSource,
	.default_version = 100,
	.default_profile = GLSLANG_NO_PROFILE,
	.force_default_version_and_profile = false,
	.forward_compatible = false,
	.messages = GLSLANG_MSG_DEFAULT_BIT,
	.resource = glslang_default_resource(),
	};

	glslang_shader_t *shader = glslang_shader_create(&input);

	EgGlslangProgram bin = {NULL, 0};
	if (!glslang_shader_preprocess(shader, &input)) {
		printf("GLSL preprocessing failed\n");
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		printf("%s\n", input.code);
		glslang_shader_delete(shader);
		return bin;
	}

	if (!glslang_shader_parse(shader, &input)) {
		printf("GLSL parsing failed\n");
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		printf("%s\n", glslang_shader_get_preprocessed_code(shader));
		glslang_shader_delete(shader);
		return bin;
	}

	glslang_program_t *program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
		printf("GLSL linking failed\n");
		printf("%s\n", glslang_program_get_info_log(program));
		printf("%s\n", glslang_program_get_info_debug_log(program));
		glslang_program_delete(program);
		glslang_shader_delete(shader);
		return bin;
	}

	glslang_program_SPIRV_generate(program, stage);

	int32_t word_size = glslang_program_SPIRV_get_size(program);
	bin.words = ecs_os_calloc(word_size * sizeof(uint32_t));
	bin.words_size = word_size;
	if (!bin.words) {
		printf("GLSL SPIR-V alloc failed\n");
		glslang_program_delete(program);
		glslang_shader_delete(shader);
		return bin;
	}
	glslang_program_SPIRV_get(program, bin.words);

	const char *spirv_messages = glslang_program_SPIRV_get_messages(program);
	if (spirv_messages)
		printf("%s\b", spirv_messages);

	glslang_program_delete(program);
	glslang_shader_delete(shader);

	return bin;
}

static void EgGlslang_create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGlslangCreate *g = ecs_field(it, EgGlslangCreate, 0); // self

	for (int i = 0; i < it->count; ++i, ++g) {
		ecs_entity_t e = it->entities[i];
		ecs_add(world, e, EgGlslangProgram);
	}
}

static void System_OnModify(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGlslangProgram *p = ecs_field(it, EgGlslangProgram, 0); // self
	EgFsContent *g = ecs_field(it, EgFsContent, 1); // shared, up
	for (int i = 0; i < it->count; ++i, ++p) {
		ecs_entity_t e = it->entities[i];
		ecs_remove(world, e, EgFsEventModify);
		if (p == NULL) {
			p = ecs_get_mut(world, e, EgGlslangProgram);
		}
		printf("System_OnModify: entity name: %s\n", ecs_get_name(world, e));
		EgGlslangProgram program = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_VERTEX, g->data);
		*p = program;
	}
}

void EgGlslangImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGlslang);
	ecs_set_name_prefix(world, "EgGlslang");

	ECS_COMPONENT_DEFINE(world, EgGlslangCreate);
	ECS_COMPONENT_DEFINE(world, EgGlslangProgram);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgGlslangProgram),
	.members = {
	{.name = "words", .type = ecs_id(ecs_uptr_t)},
	{.name = "words_size", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgGlslangCreate),
	.members = {
	{.name = "stage", .type = ecs_id(ecs_i32_t)}, // 1=vert, 4=frag, ...
	}});

	glslang_initialize_process();

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "EgGlslang_create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = EgGlslang_create,
	.immediate = true,
	.query.terms = {
	{.id = ecs_id(EgGlslangCreate)},
	{.id = ecs_id(EgGlslangProgram), .oper = EcsNot}, // Creates this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_OnModify", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_OnModify,
	.query.terms = {
	{.id = ecs_id(EgGlslangProgram), .src.id = EcsSelf},
	{.id = ecs_id(EgFsContent), .trav = EgFsEventModify, .src.id = EcsUp},
	{.id = EgFsEventModify, .src.id = EcsSelf},
	}});
}
