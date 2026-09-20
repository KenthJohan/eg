#include <EgGpusSdl_test.h>
#include <EgGpusSdl.h>
#include <EgGpus.h>
#include <EgShapes.h>
#include <SDL3/SDL_init.h>
#include <stdlib.h>

static ecs_world_t *world;

void GpuResourcesTexture_setup(void) {
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_InitSubSystem failed: %s\n", SDL_GetError());
		abort();
	}
	world = ecs_init();
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgGpusSdl);
}

void GpuResourcesTexture_teardown(void) {
	ecs_fini(world);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GpuResourcesTexture_test_texture_add_initializes_handle(void) {
	ecs_entity_t texture_entity = ecs_new(world);
	ecs_add(world, texture_entity, EgGpusTexture);

	const EgGpusTexture *texture = ecs_get(world, texture_entity, EgGpusTexture);
	test_assert(texture != NULL);
	test_assert(texture->object == NULL);
}

void GpuResourcesTexture_test_invalid_texture_size_disables_entity(void) {
	ecs_entity_t device_entity = ecs_new(world);
	ecs_add(world, device_entity, EgGpusDevice);

	ecs_entity_t texture_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add(world, texture_entity, EgGpusTexture);
	ecs_set(world, texture_entity, EgShapesRectangle, {.w = 0.0f, .h = 64.0f});

	test_assert(ecs_has_id(world, texture_entity, EcsDisabled));
}

