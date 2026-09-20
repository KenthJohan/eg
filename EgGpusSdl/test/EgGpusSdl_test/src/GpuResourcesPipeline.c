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

void GpuResourcesPipeline_setup(void) {
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

void GpuResourcesPipeline_teardown(void) {
	ecs_delete(world, vertex_shader_file);
	ecs_delete(world, fragment_shader_file);
	ecs_fini(world);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GpuResourcesPipeline_test_graphics_pipeline_create(void) {
	typedef struct {
		float x;
		float y;
	} GpuTestVertex;

	ecs_entity_t vertex_component = ecs_component_init(world,
	&(ecs_component_desc_t){
	.entity = ecs_entity(world, {.name = "GpuTestVertex"}),
	.type   = {
	.size      = sizeof(GpuTestVertex),
	.alignment = ECS_ALIGNOF(GpuTestVertex)}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = vertex_component,
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	},
	.create_member_entities = true,
	});

	ecs_entity_t ecs_entity_member_x = ecs_lookup_child(world, vertex_component, "x");
	ecs_entity_t ecs_entity_member_y = ecs_lookup_child(world, vertex_component, "y");
	ecs_set(world, ecs_entity_member_x, EgGpusLocation, {0});
	ecs_set(world, ecs_entity_member_y, EgGpusLocation, {1});

	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	ecs_entity_t vertex_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, vertex_entity, EcsDependsOn, vertex_shader_file);
	ecs_set(world, vertex_entity, EgGpusShaderCreateInfo, {.stage = EgGpusShaderStageVertex});
	ecs_entity_t fragment_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, fragment_entity, EcsDependsOn, fragment_shader_file);
	ecs_set(world, fragment_entity, EgGpusShaderCreateInfo, {.stage = EgGpusShaderStageFragment});
	ecs_progress(world, 0.0f);

	ecs_entity_t pipeline_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, vertex_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, fragment_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, vertex_component);
	test_assert(ecs_lookup_child(world, vertex_component, "x") != 0);
	test_assert(ecs_lookup_child(world, vertex_component, "y") != 0);
	ecs_set(world, pipeline_entity, EgGpusGraphicsPipelineCreateInfo, {.sample_count = 0});
	ecs_progress(world, 0.0f);

	const EgGpusGraphicsPipeline *pipeline = ecs_get(
	world, pipeline_entity, EgGpusGraphicsPipeline);
	test_assert(pipeline != NULL);
	test_assert(pipeline->object != NULL);
	test_assert(pipeline->info_num_vertex_attributes == 2);
}

void GpuResourcesPipeline_test_graphics_pipeline_create_with_position_color_uv(void) {
	typedef struct {
		float   position[3];
		uint8_t color[4];
		float   uv[2];
	} GpuTestVertexAttributes;

	ecs_entity_t vertex_component = ecs_component_init(world,
	&(ecs_component_desc_t){
	.entity = ecs_entity(world, {.name = "GpuTestVertexAttributes"}),
	.type   = {
	.size      = sizeof(GpuTestVertexAttributes),
	.alignment = ECS_ALIGNOF(GpuTestVertexAttributes),
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = vertex_component,
	.members = {
	{.name = "position", .type = ecs_id(ecs_f32_t), .count = 3},
	{.name = "color", .type = ecs_id(ecs_u8_t), .count = 4},
	{.name = "uv", .type = ecs_id(ecs_f32_t), .count = 2},
	},
	.create_member_entities = true,
	});

	ecs_entity_t position_member = ecs_lookup_child(world, vertex_component, "position");
	ecs_entity_t color_member    = ecs_lookup_child(world, vertex_component, "color");
	ecs_entity_t uv_member       = ecs_lookup_child(world, vertex_component, "uv");
	ecs_set(world, position_member, EgGpusLocation, {0});
	ecs_set(world, color_member, EgGpusLocation, {1});
	ecs_set(world, uv_member, EgGpusLocation, {2});

	ecs_entity_t device_entity = ecs_new(world);
	ecs_set(world, device_entity, EgGpusDeviceCreateInfo, {0});
	ecs_progress(world, 0.0f);

	ecs_entity_t vertex_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, vertex_entity, EcsDependsOn, vertex_shader_file);
	ecs_set(world, vertex_entity, EgGpusShaderCreateInfo, {.stage = EgGpusShaderStageVertex});
	ecs_entity_t fragment_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, fragment_entity, EcsDependsOn, fragment_shader_file);
	ecs_set(world, fragment_entity, EgGpusShaderCreateInfo, {.stage = EgGpusShaderStageFragment});
	ecs_progress(world, 0.0f);

	ecs_entity_t pipeline_entity = ecs_new_w_pair(world, EcsChildOf, device_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, vertex_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, fragment_entity);
	ecs_add_pair(world, pipeline_entity, EcsDependsOn, vertex_component);
	test_assert(ecs_lookup_child(world, vertex_component, "position") != 0);
	test_assert(ecs_lookup_child(world, vertex_component, "color") != 0);
	test_assert(ecs_lookup_child(world, vertex_component, "uv") != 0);
	ecs_set(world, pipeline_entity, EgGpusGraphicsPipelineCreateInfo, {.sample_count = 0});
	ecs_progress(world, 0.0f);

	const EgGpusGraphicsPipeline *pipeline = ecs_get(
	world, pipeline_entity, EgGpusGraphicsPipeline);
	test_assert(pipeline != NULL);
	test_assert(pipeline->object != NULL);
	test_assert(pipeline->info_num_vertex_attributes == 3);
}

