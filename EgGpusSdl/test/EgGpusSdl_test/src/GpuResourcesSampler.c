#include <EgGpusSdl_test.h>
#include <EgGpusSdl.h>
#include <EgGpus.h>
#include <SDL3/SDL_init.h>
#include <stdlib.h>

static ecs_world_t *world;

void GpuResourcesSampler_setup(void) {
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_InitSubSystem failed: %s\n", SDL_GetError());
		abort();
	}
	world = ecs_init();
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgGpusSdl);
}

void GpuResourcesSampler_teardown(void) {
	ecs_fini(world);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GpuResourcesSampler_test_create(void) {
	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	ecs_entity_t sampler_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_set(world, sampler_entity, EgGpusSamplerCreateInfo, {
	.min_filter = EgGpusSamplerFilterNearest,
	.mag_filter = EgGpusSamplerFilterNearest,
	.address_mode_u = EgGpusSamplerAddressModeClampToEdge,
	.address_mode_v = EgGpusSamplerAddressModeClampToEdge,
	.address_mode_w = EgGpusSamplerAddressModeClampToEdge,
	});
	ecs_progress(world, 0.0f);

	const EgGpusSampler *sampler = ecs_get(world, sampler_entity, EgGpusSampler);
	test_assert(sampler != NULL);
	test_assert(sampler->object != NULL);
}
