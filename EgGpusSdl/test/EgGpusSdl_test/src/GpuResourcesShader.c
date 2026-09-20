#include <EgGpusSdl_test.h>
#include <EgGpusSdl.h>
#include <EgGpus.h>
#include <EgFs.h>
#include <EgShapes.h>
#include <ecsx/ecsx_file.h>
#include <SDL3/SDL_init.h>
#include <stdlib.h>

static ecs_world_t *world;
static ecs_entity_t vertex_shader_file;
static ecs_entity_t fragment_shader_file;

void GpuResourcesShader_setup(void) {
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_InitSubSystem failed: %s\n", SDL_GetError());
		abort();
	}
	world = ecs_init();
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgGpusSdl);

	size_t length = 0;
	char *content = ecsx_file_load_alloc("data/vertex.spv", &length);
	vertex_shader_file = ecs_new(world);
	EgFsContent vertex_content = {.data = content, .size = (uint32_t)length};
	ecs_set_ptr(world, vertex_shader_file, EgFsContent, &vertex_content);

	length = 0;
	content = ecsx_file_load_alloc("data/fragment.spv", &length);
	fragment_shader_file = ecs_new(world);
	EgFsContent fragment_content = {.data = content, .size = (uint32_t)length};
	ecs_set_ptr(world, fragment_shader_file, EgFsContent, &fragment_content);
}

void GpuResourcesShader_teardown(void) {
	ecs_delete(world, vertex_shader_file);
	ecs_delete(world, fragment_shader_file);
	ecs_fini(world);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GpuResourcesShader_test_vertex_shader_create(void) {
	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	const EgGpusDevice *device = ecs_get(world, device_entity, EgGpusDevice);
	test_assert(device != NULL);
	test_assert(device->object != NULL);

	ecs_entity_t shader_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, shader_entity, EcsDependsOn, vertex_shader_file);
	ecs_set(world, shader_entity, EgGpusShaderCreateInfo, {.stage = EgGpusShaderStageVertex});
	ecs_progress(world, 0.0f);

	const EgGpusShader *shader = ecs_get(world, shader_entity, EgGpusShader);
	test_assert(shader != NULL);
	test_assert(shader->object != NULL);
}

void GpuResourcesShader_test_fragment_shader_create(void) {
	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	const EgGpusDevice *device = ecs_get(world, device_entity, EgGpusDevice);
	test_assert(device != NULL);
	test_assert(device->object != NULL);

	ecs_entity_t shader_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, shader_entity, EcsDependsOn, fragment_shader_file);
	ecs_set(world, shader_entity, EgGpusShaderCreateInfo, {.stage = EgGpusShaderStageFragment});
	ecs_progress(world, 0.0f);

	const EgGpusShader *shader = ecs_get(world, shader_entity, EgGpusShader);
	test_assert(shader != NULL);
	test_assert(shader->object != NULL);
}

