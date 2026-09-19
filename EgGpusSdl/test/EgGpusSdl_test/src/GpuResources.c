#include <EgGpusSdl_test.h>
#include <EgGpusSdl.h>
#include <EgGpus.h>
#include <EgShapes.h>
#include <SDL3/SDL_init.h>
#include <stdlib.h>

static ecs_world_t *world;

typedef struct {
	float x;
	float y;
} GpuTestVertex;

ECS_COMPONENT_DECLARE(GpuTestVertex);

void GpuResources_setup(void)
{
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_InitSubSystem failed: %s\n", SDL_GetError());
		abort();
	}
	world = ecs_init();
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgGpusSdl);
	ECS_COMPONENT_DEFINE(world, GpuTestVertex);
	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(GpuTestVertex),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	},
	.create_member_entities = true
	});
	ecs_progress(world, 0.0f);
}

void GpuResources_teardown(void)
{
	ecs_fini(world);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GpuResources_test_import(void)
{
	test_assert(ecs_id(EgGpusDevice) != 0);
	test_assert(ecs_id(EgGpusTexture) != 0);
}

void GpuResources_test_device_add_initializes_handle(void)
{
	ecs_entity_t device_entity = ecs_new(world);
	ecs_add(world, device_entity, EgGpusDevice);

	const EgGpusDevice *device = ecs_get(world, device_entity, EgGpusDevice);
	test_assert(device != NULL);
	test_assert(device->object == NULL);
}

void GpuResources_test_texture_add_initializes_handle(void)
{
	ecs_entity_t texture_entity = ecs_new(world);
	ecs_add(world, texture_entity, EgGpusTexture);

	const EgGpusTexture *texture = ecs_get(world, texture_entity, EgGpusTexture);
	test_assert(texture != NULL);
	test_assert(texture->object == NULL);
}

void GpuResources_test_invalid_texture_size_disables_entity(void)
{
	ecs_entity_t device_entity = ecs_new(world);
	ecs_add(world, device_entity, EgGpusDevice);

	ecs_entity_t texture_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add(world, texture_entity, EgGpusTexture);
	ecs_set(world, texture_entity, EgShapesRectangle, {.w = 0.0f, .h = 64.0f});

	test_assert(ecs_has_id(world, texture_entity, EcsDisabled));
}

void GpuResources_test_vertex_shader_create(void)
{
	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	const EgGpusDevice *device = ecs_get(world, device_entity, EgGpusDevice);
	test_assert(device != NULL);
	test_assert(device->object != NULL);

	ecs_entity_t shader_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_set(world, shader_entity, EgGpusShaderVertexCreateInfo, {.path = "data/vertex.spv"});
	ecs_progress(world, 0.0f);

	const EgGpusShaderVertex *shader = ecs_get(world, shader_entity, EgGpusShaderVertex);
	test_assert(shader != NULL);
	test_assert(shader->object != NULL);
}

void GpuResources_test_fragment_shader_create(void)
{
	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	const EgGpusDevice *device = ecs_get(world, device_entity, EgGpusDevice);
	test_assert(device != NULL);
	test_assert(device->object != NULL);

	ecs_entity_t shader_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_set(world, shader_entity, EgGpusShaderFragmentCreateInfo, {.path = "data/fragment.spv"});
	ecs_progress(world, 0.0f);

	const EgGpusShaderFragment *shader = ecs_get(world, shader_entity, EgGpusShaderFragment);
	test_assert(shader != NULL);
	test_assert(shader->object != NULL);
}

void GpuResources_test_graphics_pipeline_create(void)
{
	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	ecs_entity_t vertex_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_set(world, vertex_entity, EgGpusShaderVertexCreateInfo, {.path = "data/vertex.spv"});
	ecs_entity_t fragment_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_set(world, fragment_entity, EgGpusShaderFragmentCreateInfo, {.path = "data/fragment.spv"});
	ecs_progress(world, 0.0f);

	ecs_entity_t pipeline_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, vertex_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, fragment_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, ecs_id(GpuTestVertex));
	test_assert(ecs_lookup_child(world, ecs_id(GpuTestVertex), "x") != 0);
	test_assert(ecs_lookup_child(world, ecs_id(GpuTestVertex), "y") != 0);
	ecs_set(world, pipeline_entity, EgGpusGraphicsPipelineCreateInfo, {.sample_count = 0});
	ecs_progress(world, 0.0f);

	const EgGpusGraphicsPipeline *pipeline = ecs_get(
	world, pipeline_entity, EgGpusGraphicsPipeline);
	test_assert(pipeline != NULL);
	test_assert(pipeline->object != NULL);
	test_assert(pipeline->info_num_vertex_attributes == 2);
}
