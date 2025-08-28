/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
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

static void EgGlslang_create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGlslangCreate *g = ecs_field(it, EgGlslangCreate, 0); // self
	EgFsContent *c = ecs_field(it, EgFsContent, 1);         // shared

	for (int i = 0; i < it->count; ++i, ++c) {
		ecs_entity_t e = it->entities[i];
		char const *name = ecs_get_name(world, e);
		char const *code = ecs_vec_first_t(&c->buf, char);
		const glslang_input_t input =
		{
		.language = GLSLANG_SOURCE_GLSL,
		.stage = GLSLANG_STAGE_VERTEX,
		.client = GLSLANG_CLIENT_VULKAN,
		.client_version = GLSLANG_TARGET_VULKAN_1_1,
		.target_language = GLSLANG_TARGET_SPV,
		.target_language_version = GLSLANG_TARGET_SPV_1_3,
		.code = code,
		.default_version = 100,
		.default_profile = GLSLANG_NO_PROFILE,
		.force_default_version_and_profile = false,
		.forward_compatible = false,
		.messages = GLSLANG_MSG_DEFAULT_BIT,
		.resource = glslang_default_resource(),
		};

		glslang_shader_t *shader = glslang_shader_create(&input);

		if (!glslang_shader_preprocess(shader, &input)) {
			// use glslang_shader_get_info_log() and glslang_shader_get_info_debug_log()
			const char *info_log = glslang_shader_get_info_log(shader);
			const char *debug_log = glslang_shader_get_info_debug_log(shader);
			ecs_err("Failed to preprocess shader: %s\n%s", info_log, debug_log);
			ecs_enable(world, e, false);
			continue;
		}

		if (!glslang_shader_parse(shader, &input)) {
			// use glslang_shader_get_info_log() and glslang_shader_get_info_debug_log()
			const char *info_log = glslang_shader_get_info_log(shader);
			const char *debug_log = glslang_shader_get_info_debug_log(shader);
			ecs_err("Failed to parse shader: %s\n%s", info_log, debug_log);
			ecs_enable(world, e, false);
			continue;
		}

		glslang_program_t *program = glslang_program_create();
		glslang_program_add_shader(program, shader);

		if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
			ecs_err("%s\n", glslang_program_get_info_log(program));
			ecs_err("%s\n", glslang_program_get_info_debug_log(program));
			glslang_program_delete(program);
			glslang_shader_delete(shader);
			ecs_enable(world, e, false);
			continue;
		}

		EgGlslangProgram p;

		glslang_program_SPIRV_generate(program, input.stage);
		p.words_size = glslang_program_SPIRV_get_size(program);
		p.words = malloc(p.words_size * sizeof(uint32_t));
		glslang_program_SPIRV_get(program, p.words);

		ecs_set_ptr(world, e, EgGlslangProgram, &p);
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

	glslang_initialize_process();

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "EgGlslang_create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = EgGlslang_create,
	.query.terms = {
	{.id = ecs_id(EgGlslangCreate)},
	{.id = ecs_id(EgFsContent), .trav = EcsDependsOn, .src.id = EcsUp, .inout = EcsIn},
	{.id = EgFsEof, .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGlslangProgram), .oper = EcsNot}, // Creates this
	}});
}
