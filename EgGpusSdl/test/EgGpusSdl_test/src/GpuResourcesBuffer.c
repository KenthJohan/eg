#include <EgGpusSdl_test.h>
#include <EgGpusSdl.h>
#include <EgGpus.h>
#include <SDL3/SDL_init.h>
#include <stdlib.h>

static ecs_world_t *world;

void GpuResourcesBuffer_setup(void) {
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_InitSubSystem failed: %s\n", SDL_GetError());
		abort();
	}
	world = ecs_init();
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgGpusSdl);
}

void GpuResourcesBuffer_teardown(void) {
	ecs_fini(world);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GpuResourcesBuffer_test_create(void) {
	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	ecs_entity_t buffer_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_set(world, buffer_entity, EgGpusBufferCreateInfo, {.size = 64, .usage = EgGpusBufferUsageVertex});
	ecs_progress(world, 0.0f);

	const EgGpusBuffer *buffer = ecs_get(world, buffer_entity, EgGpusBuffer);
	test_assert(buffer != NULL);
	test_assert(buffer->object != NULL);
}
