#include "egsokol.h"
#include <sokol_shape.h>
#include <stdio.h>
#include <stdlib.h>

ECS_COMPONENT_DECLARE(SgPipelineCreate);
ECS_COMPONENT_DECLARE(SgPipeline);
ECS_COMPONENT_DECLARE(SgShaderCreate);
ECS_COMPONENT_DECLARE(SgShader);
ECS_COMPONENT_DECLARE(SgLocation);
ECS_COMPONENT_DECLARE(SgVertexBufferLayout);
ECS_COMPONENT_DECLARE(SgAttributes);
ECS_COMPONENT_DECLARE(SgUniformBlocks);
ECS_COMPONENT_DECLARE(SgVertexFormat);
ECS_COMPONENT_DECLARE(SgUniformType);
ECS_COMPONENT_DECLARE(SgIndexType);
ECS_COMPONENT_DECLARE(SgPrimitiveType);
ECS_COMPONENT_DECLARE(SgCullMode);
ECS_COMPONENT_DECLARE(SgUniformBlock);
ECS_COMPONENT_DECLARE(SgUniform);

#define ENTITY_COLOR "#55A3F4"

char *eg_fs_readfile(char const *path)
{
	ecs_assert(path != NULL, ECS_INTERNAL_ERROR, NULL);
	char *content = NULL;

	FILE *file = fopen(path, "r");
	if (file == NULL) {
		goto error;
	}

	fseek(file, 0, SEEK_END);
	int32_t size = (int32_t)ftell(file);

	if (size == -1) {
		goto error;
	}
	rewind(file);

	content = malloc(size + 1);
	content[size] = '\0';
	size_t n = fread(content, size, 1, file);
	if (n != 1) {
		ecs_dbg_2("%s: could not read wholef file %d bytes\n", path, size);
		goto error;
	}
	ecs_assert(content[size] == '\0', ECS_INTERNAL_ERROR, "Expected null terminator");
	fclose(file);
	return content;
error:
	if (content) {
		free(content);
	}
	return NULL;
}

static void print_entity(ecs_world_t *world, ecs_entity_t e)
{
	ecs_entity_t scope = ecs_get_scope(world);
	char const *scope_name = scope ? ecs_get_name(world, scope) : "";
	char *path_str = ecs_get_path(world, e);
	char *type_str = ecs_type_str(world, ecs_get_type(world, e));
	ecs_dbg_2(ECS_GREY "%s" ECS_NORMAL " %s [%s]\n", scope_name, path_str, type_str);
	ecs_os_free(type_str);
	ecs_os_free(path_str);
}

static void print_entity_from_it(ecs_iter_t *it, int i)
{
	ecs_entity_t s = it->system ? ecs_get_parent(it->world, it->system) : 0;
	ecs_entity_t parent = s ? ecs_get_parent(it->world, s) : ecs_get_scope(it->world);
	char const *scope_name = parent ? ecs_get_name(it->world, parent) : "";
	char *path_str = ecs_get_path(it->world, it->entities[i]);
	char *type_str = ecs_type_str(it->world, ecs_get_type(it->world, it->entities[i]));
	ecs_dbg_2(ECS_MAGENTA "%s" ECS_NORMAL " %s [%s]\n", scope_name, path_str, type_str);
	ecs_os_free(type_str);
	ecs_os_free(path_str);
}

static void iterate_children(ecs_world_t *world, ecs_entity_t parent)
{
	ecs_iter_t it = ecs_children(world, parent);
	while (ecs_children_next(&it)) {
		for (int i = 0; i < it.count; i++) {
			ecs_entity_t e = it.entities[i];
			print_entity(world, e);
		}
	}
}

static void iterate_shader_attrs(ecs_world_t *world, ecs_entity_t parent, sg_shader_attr_desc *descs)
{
	ecs_iter_t it = ecs_children(world, parent);
	while (ecs_children_next(&it)) {
		for (int i = 0; i < it.count; i++) {
			ecs_entity_t e = it.entities[i];
			ecs_doc_set_color(world, e, ENTITY_COLOR);
			char const *name = ecs_get_name(world, e);
			SgLocation const *loc = ecs_get(world, e, SgLocation);
			descs[loc->index].name = name;
		}
	}
}

static void iterate_shader_uniforms(ecs_world_t *world, ecs_entity_t parent, sg_shader_uniform_desc *descs)
{
	ecs_iter_t it = ecs_children(world, parent);
	while (ecs_children_next(&it)) {
		for (int i = 0; i < it.count; i++) {
			ecs_entity_t e = it.entities[i];
			ecs_doc_set_color(world, e, ENTITY_COLOR);
			print_entity_from_it(&it, i);
			char const *name = ecs_get_name(world, e);
			SgUniform const *uniform = ecs_get(world, e, SgUniform);
			ecs_assert(uniform != NULL, ECS_INTERNAL_ERROR, NULL);
			descs[uniform->index].name = name;
			descs[uniform->index].array_count = uniform->array_count;
			descs[uniform->index].type = (sg_uniform_type)uniform->type;
		}
	}
}

static void iterate_shader_blocks(ecs_world_t *world, ecs_entity_t parent, sg_shader_uniform_block_desc *descs)
{
	ecs_iter_t it = ecs_children(world, parent);
	while (ecs_children_next(&it)) {
		for (int i = 0; i < it.count; i++) {
			ecs_entity_t e = it.entities[i];
			ecs_doc_set_color(world, e, ENTITY_COLOR);
			print_entity_from_it(&it, i);
			// char const * name = ecs_get_name(world, e);
			ecs_i32_t index = ecs_get(world, e, SgUniformBlock)->index;
			ecs_i32_t size = ecs_get(world, e, SgUniformBlock)->size;
			descs[index].size = size;
			descs[index].layout = SG_UNIFORMLAYOUT_STD140;
			iterate_shader_uniforms(world, e, descs[index].uniforms);
		}
	}
}

static void iterate_vertex_attrs(ecs_world_t *world, ecs_entity_t parent, sg_vertex_attr_state *descs)
{
	ecs_iter_t it = ecs_children(world, parent);
	while (ecs_children_next(&it)) {
		for (int i = 0; i < it.count; i++) {
			ecs_entity_t e = it.entities[i];
			print_entity_from_it(&it, i);
			ecs_doc_set_color(world, e, ENTITY_COLOR);
			SgLocation *loc = ecs_get_mut(world, e, SgLocation);
			sg_vertex_attr_state *outstate = descs + loc->index;
			outstate->offset = loc->offset;
			outstate->buffer_index = loc->buffer_index;
			outstate->format = (sg_vertex_format)loc->format;
		}
	}
}

static void set_vertex_buffers(ecs_world_t *world, ecs_entity_t parent, sg_vertex_buffer_layout_state buffers[SG_MAX_VERTEX_BUFFERS])
{
	char buf[8];
	for (int i = 0; i < SG_MAX_VERTEX_BUFFERS; ++i) {
		snprintf(buf, sizeof(buf), "buf%i", i);
		ecs_entity_t e = ecs_lookup_child(world, parent, buf);
		ecs_log(0, "%s:%s:%s", ecs_get_name(world, parent), buf, e ? ecs_get_name(world, e) : "");
		if (e == 0) {
			continue;
		}
		SgVertexBufferLayout const *b = ecs_get(world, e, SgVertexBufferLayout);
		if (b == NULL) {
			continue;
		}
		buffers[i].stride = b->stride;
		buffers[i].step_func = b->step_func;
		buffers[i].step_rate = b->step_rate;
	}
}

static void Pip_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	SgPipelineCreate *create = ecs_field(it, SgPipelineCreate, 0); // self

	// SgAttributes * attrs = ecs_field(it, SgAttributes, 1); // up
	// ecs_entity_t entity_attrs = ecs_field_src(it, 1);
	// SgShader *shader = ecs_field(it, SgShader, 2); // up
	// ecs_doc_set_color(world, entity_attrs, ENTITY_COLOR);

	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];

		// ecs_log_set_level(2);
		// print_entity_from_it(it, i);
		// ecs_log_set_level(0);

		if (create->shader == 0) {
			ecs_warn("No shader entity");
			return;
		}
		SgShader const *shader = ecs_get(world, create->shader, SgShader);
		if (shader == NULL) {
			// ecs_warn("No SgShader component in shader entity");
			return;
		}
		SgShaderCreate const *shaderinfo = ecs_get(world, create->shader, SgShaderCreate);
		if (shaderinfo == NULL) {
			ecs_warn("No SgShaderCreate component in shader entity");
			return;
		}

		ecs_doc_set_color(world, e, ENTITY_COLOR);

		SgPipeline *pip = ecs_ensure(world, e, SgPipeline);

		sg_pipeline_desc desc = {
		.shader = (sg_shader){shader->id},
		.depth = {.write_enabled = true, .compare = SG_COMPAREFUNC_LESS_EQUAL},
		.index_type = (sg_index_type)create->indextype,
		.primitive_type = (sg_primitive_type)create->primtype,
		.cull_mode = (sg_cull_mode)create->cullmode,
		// TODO: Use 1 when for offscreen rendering
		// Use 4 when render directly
		.sample_count = 1,
		};
		iterate_vertex_attrs(world, shaderinfo->attrs, desc.layout.attrs);
		set_vertex_buffers(world, e, desc.layout.buffers);

		pip->id = sg_make_pipeline(&desc).id;
	}
}

// https://github.com/SanderMertens/flecs/blob/ca73ed213310f2ca23f2afde38f72af793091e50/examples/c/entities/hierarchy/src/main.c#L52
static void Shader_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	SgShaderCreate *create = ecs_field(it, SgShaderCreate, 0);
	if (create->attrs == 0) {
		ecs_warn("No attrs entity");
		return;
	}
	if (create->ubs == 0) {
		ecs_warn("No ubs entity");
		return;
	}
	/*
	ecs_entity_t entity_attrs = ecs_field_src(it, 1);
	ecs_entity_t entity_blocks = ecs_field_src(it, 2);
	ecs_doc_set_color(world, entity_attrs, ENTITY_COLOR);
	ecs_doc_set_color(world, entity_blocks, ENTITY_COLOR);
	*/

	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		print_entity_from_it(it, i);
		ecs_doc_set_color(world, e, "#003366");
		sg_shader_desc desc = {0};

		ecs_dbg_2("Readfile %s\n", create->filename_vs);
		desc.vs.source = eg_fs_readfile(create->filename_vs);
		if (desc.vs.source == NULL) {
			ecs_enable(world, e, false);
			break;
		}

		ecs_dbg_2("Readfile %s\n", create->filename_fs);
		desc.fs.source = eg_fs_readfile(create->filename_fs);
		if (desc.fs.source == NULL) {
			ecs_enable(world, e, false);
			break;
		}

		desc.vs.entry = "main";
		desc.fs.entry = "main";

		ecs_assert(desc.vs.source != NULL, ECS_INTERNAL_ERROR, NULL);
		ecs_assert(desc.fs.source != NULL, ECS_INTERNAL_ERROR, NULL);
		// shader->id = create_shader(desc->filename_fs, desc->filename_vs);
		iterate_shader_attrs(world, create->attrs, desc.attrs);
		iterate_shader_blocks(world, create->ubs, desc.vs.uniform_blocks);

		sg_shader shd = sg_make_shader(&desc);
		SgShader *shader = ecs_ensure(world, e, SgShader);
		shader->id = shd.id;
		ecs_dbg_2("");
	}
}

void SgImport(ecs_world_t *world)
{
	ECS_MODULE(world, Sg);

	ecs_set_name_prefix(world, "Sg");
	ECS_COMPONENT_DEFINE(world, SgPipelineCreate);
	ECS_COMPONENT_DEFINE(world, SgPipeline);
	ECS_COMPONENT_DEFINE(world, SgShaderCreate);
	ECS_COMPONENT_DEFINE(world, SgShader);
	ECS_COMPONENT_DEFINE(world, SgLocation);
	ECS_COMPONENT_DEFINE(world, SgVertexBufferLayout);
	ECS_COMPONENT_DEFINE(world, SgAttributes);
	ECS_COMPONENT_DEFINE(world, SgUniformBlocks);
	ECS_COMPONENT_DEFINE(world, SgVertexFormat);
	ECS_COMPONENT_DEFINE(world, SgUniformType);
	ECS_COMPONENT_DEFINE(world, SgIndexType);
	ECS_COMPONENT_DEFINE(world, SgPrimitiveType);
	ECS_COMPONENT_DEFINE(world, SgCullMode);
	ECS_COMPONENT_DEFINE(world, SgUniformBlock);
	ECS_COMPONENT_DEFINE(world, SgUniform);

	// clang-format off
	ecs_enum(world, {.entity = ecs_id(SgPrimitiveType), .constants = {
		{.name = "DEFAULT", .value = SgPrimitiveTypeDEFAULT},
		{.name = "POINTS", .value = SgPrimitiveTypePOINTS},
		{.name = "LINES", .value = SgPrimitiveTypeLINES},
		{.name = "LINE_STRIP", .value = SgPrimitiveTypeLINE_STRIP},
		{.name = "TRIANGLES", .value = SgPrimitiveTypeTRIANGLES},
		{.name = "TRIANGLE_STRIP", .value = SgPrimitiveTypeTRIANGLE_STRIP},
		{.name = "NUM", .value = SgPrimitiveTypeNUM},
		{.name = "FORCE_U32", .value = SgPrimitiveTypeFORCE_U32},
		}});

	ecs_enum(world, {.entity = ecs_id(SgCullMode), .constants = {
		{.name = "DEFAULT", .value = SgCullModeDEFAULT},
		{.name = "NONE", .value = SgCullModeNONE},
		{.name = "FRONT", .value = SgCullModeFRONT},
		{.name = "BACK", .value = SgCullModeBACK},
		{.name = "NUM", .value = SgCullModeNUM},
		{.name = "FORCE_U32", .value = SgCullModeFORCE_U32},
		}});

	ecs_enum(world, {.entity = ecs_id(SgIndexType), .constants = {
		{.name = "DEFAULT", .value = SgIndexTypeDEFAULT},
		{.name = "NONE", .value = SgIndexTypeNONE},
		{.name = "UINT16", .value = SgIndexTypeUINT16},
		{.name = "UINT32", .value = SgIndexTypeUINT32},
		{.name = "NUM", .value = SgIndexTypeNUM},
		{.name = "FORCE_U32", .value = SgIndexTypeFORCE_U32},
		}});

	ecs_enum(world, {.entity = ecs_id(SgVertexFormat), .constants = {
		{.name = "INVALID", .value = SgVertexFormatINVALID},
		{.name = "FLOAT", .value = SgVertexFormatFLOAT},
		{.name = "FLOAT2", .value = SgVertexFormatFLOAT2},
		{.name = "FLOAT3", .value = SgVertexFormatFLOAT3},
		{.name = "FLOAT4", .value = SgVertexFormatFLOAT4},
		{.name = "BYTE4", .value = SgVertexFormatBYTE4},
		{.name = "BYTE4N", .value = SgVertexFormatBYTE4N},
		{.name = "UBYTE4", .value = SgVertexFormatUBYTE4},
		{.name = "UBYTE4N", .value = SgVertexFormatUBYTE4N},
		{.name = "SHORT2", .value = SgVertexFormatSHORT2},
		{.name = "SHORT2N", .value = SgVertexFormatSHORT2N},
		{.name = "USHORT2N", .value = SgVertexFormatUSHORT2N},
		{.name = "SHORT4", .value = SgVertexFormatSHORT4},
		{.name = "SHORT4N", .value = SgVertexFormatSHORT4N},
		{.name = "USHORT4N", .value = SgVertexFormatUSHORT4N},
		{.name = "UINT10_N2", .value = SgVertexFormatUINT10_N2},
		{.name = "HALF2", .value = SgVertexFormatHALF2},
		{.name = "HALF4", .value = SgVertexFormatHALF4},
		{.name = "NUM", .value = SgVertexFormatNUM},
		{.name = "FORCE_U32", .value = SgVertexFormatFORCE_U32},
		}});

	ecs_enum(world, {.entity = ecs_id(SgUniformType), .constants = {
		{.name = "INVALID", .value = SgUniformTypeINVALID},
		{.name = "FLOAT", .value = SgUniformTypeFLOAT},
		{.name = "FLOAT2", .value = SgUniformTypeFLOAT2},
		{.name = "FLOAT3", .value = SgUniformTypeFLOAT3},
		{.name = "FLOAT4", .value = SgUniformTypeFLOAT4},
		{.name = "INT", .value = SgUniformTypeINT},
		{.name = "INT2", .value = SgUniformTypeINT2},
		{.name = "INT3", .value = SgUniformTypeINT3},
		{.name = "INT4", .value = SgUniformTypeINT4},
		{.name = "MAT4", .value = SgUniformTypeMAT4},
		{.name = "NUM", .value = SgUniformTypeNUM},
		{.name = "FORCE_U32", .value = SgUniformTypeFORCE_U32},
		}});
	// clang-format on

	{
		sg_vertex_attr_state a0 = sshape_position_vertex_attr_state();
		sg_vertex_attr_state a1 = sshape_normal_vertex_attr_state();
		sg_vertex_attr_state a2 = sshape_texcoord_vertex_attr_state();
		sg_vertex_attr_state a3 = sshape_color_vertex_attr_state();

		sg_vertex_buffer_layout_state l = sshape_vertex_buffer_layout_state();

		ecs_dbg_2("");
	}

	ecs_struct(world,
	{.entity = ecs_id(SgPipelineCreate),
	.members = {
	{.name = "shader", .type = ecs_id(ecs_entity_t)},
	{.name = "primtype", .type = ecs_id(SgPrimitiveType)},
	{.name = "cullmode", .type = ecs_id(SgCullMode)},
	{.name = "indextype", .type = ecs_id(SgIndexType)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgShaderCreate),
	.members = {
	{.name = "filename_vs", .type = ecs_id(ecs_string_t)},
	{.name = "filename_fs", .type = ecs_id(ecs_string_t)},
	{.name = "ubs", .type = ecs_id(ecs_entity_t)},
	{.name = "attrs", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgShader),
	.members = {
	{.name = "id", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgPipeline),
	.members = {
	{.name = "id", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgVertexBufferLayout),
	.members = {
	{.name = "stride", .type = ecs_id(ecs_i32_t)},
	{.name = "step_func", .type = ecs_id(ecs_i32_t)},
	{.name = "step_rate", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgLocation),
	.members = {
	{.name = "index", .type = ecs_id(ecs_i32_t)},
	{.name = "format", .type = ecs_id(SgVertexFormat)},
	{.name = "offset", .type = ecs_id(ecs_i32_t)},
	{.name = "buffer_index", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgUniformBlock),
	.members = {
	{.name = "index", .type = ecs_id(ecs_i32_t)},
	{.name = "size", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgUniform),
	.members = {
	{.name = "index", .type = ecs_id(ecs_i32_t)},
	{.name = "array_count", .type = ecs_id(ecs_i32_t)},
	{.name = "type", .type = ecs_id(SgUniformType)},
	}});

	// clang-format off
	ecs_system(world, {.entity = ecs_entity(world, {.name = "Pip_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
		.callback = Pip_Create,
		.query.terms =
		{
		{.id = ecs_id(SgPipelineCreate), .src.id = EcsSelf},
		{.id = ecs_id(SgPipeline), .oper = EcsNot}, // Adds this
		}});

	ecs_system(world, {.entity = ecs_entity(world, {.name = "Shader_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
		.callback = Shader_Create,
		.query.terms =
		{
		{.id = ecs_id(SgShaderCreate), .src.id = EcsSelf},
		{.id = ecs_id(SgShader), .oper = EcsNot}, // Adds this
		}});
	// clang-format on
}

void egsokol_flecs_event_cb(const sapp_event *evt, Window *window)
{
	uint8_t *keys = window->keys;

	switch (evt->type) {
	case SAPP_EVENTTYPE_MOUSE_DOWN:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			if (window->mouse_left == 0) {
				window->mouse_left_edge = 1;
			}
			window->mouse_left = 1;
		}

		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		}
		break;
	case SAPP_EVENTTYPE_MOUSE_UP:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			window->mouse_left = 0;
		}

		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		}
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		break;
	case SAPP_EVENTTYPE_MOUSE_MOVE:
		window->mouse_x = evt->mouse_x;
		window->mouse_y = evt->mouse_y;
		window->mouse_dx = evt->mouse_dx;
		window->mouse_dy = evt->mouse_dy;
		break;
	case SAPP_EVENTTYPE_KEY_UP:
		assert(evt->key_code < 512);
		keys[evt->key_code] = 0;
		break;
	case SAPP_EVENTTYPE_KEY_DOWN:
		assert(evt->key_code < 512);
		keys[evt->key_code] = 1;
		break;
	case SAPP_EVENTTYPE_RESIZED: {
		break;
	}
	default:
		break;
	}

	if (keys[SAPP_KEYCODE_ESCAPE]) {
		sapp_quit();
	}
}