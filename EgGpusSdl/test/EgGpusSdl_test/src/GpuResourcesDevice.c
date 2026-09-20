#include <EgGpusSdl_test.h>
#include <EgGpusSdl.h>
#include <EgGpus.h>
#include <SDL3/SDL_init.h>
#include <stdlib.h>

static ecs_world_t *world;

void GpuResourcesDevice_setup(void) {
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_InitSubSystem failed: %s\n", SDL_GetError());
		abort();
	}
	world = ecs_init();
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgGpusSdl);
}

void GpuResourcesDevice_teardown(void) {
	ecs_fini(world);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GpuResourcesDevice_test_import(void) {
	test_assert(ecs_id(EgGpusDevice) != 0);
	test_assert(ecs_id(EgGpusTexture) != 0);
}

void GpuResourcesDevice_test_device_add_initializes_handle(void) {
	ecs_entity_t device_entity = ecs_new(world);
	ecs_add(world, device_entity, EgGpusDevice);

	const EgGpusDevice *device = ecs_get(world, device_entity, EgGpusDevice);
	test_assert(device != NULL);
	test_assert(device->object == NULL);
}

