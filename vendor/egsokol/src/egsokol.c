#include "egsokol.h"
#include <egcomponents.h>
//#include <eg/eg_assert.h>
//#include <eg/eg_fs.h>
//#include <eg/eg_log.h>
#include <sokol_shape.h>
#include <stdio.h>
#include <stdlib.h>

ECS_COMPONENT_DECLARE(SgPipelineCreate);
ECS_COMPONENT_DECLARE(SgPipeline);
ECS_COMPONENT_DECLARE(SgShaderCreate);
ECS_COMPONENT_DECLARE(SgShader);
ECS_COMPONENT_DECLARE(SgLocation);
ECS_COMPONENT_DECLARE(SgAttribute);
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

ECS_TAG_DECLARE(SgNone);
ECS_TAG_DECLARE(SgPoints);
ECS_TAG_DECLARE(SgLines);
ECS_TAG_DECLARE(SgTriangles);
ECS_TAG_DECLARE(SgFloat2);
ECS_TAG_DECLARE(SgFloat3);
ECS_TAG_DECLARE(SgFloat4);
ECS_TAG_DECLARE(SgByte4n);
ECS_TAG_DECLARE(SgUbyte4n);
ECS_TAG_DECLARE(SgUshort2n);
ECS_TAG_DECLARE(SgU16);
ECS_TAG_DECLARE(SgU32);
ECS_TAG_DECLARE(SgFront);
ECS_TAG_DECLARE(SgBack);

ECS_TAG_DECLARE(SgAttributeShapePosition);
ECS_TAG_DECLARE(SgAttributeShapeNormal);
ECS_TAG_DECLARE(SgAttributeShapeTextcoord);
ECS_TAG_DECLARE(SgAttributeShapeColor);
ECS_TAG_DECLARE(SgVertexBufferLayoutShape);

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

static void const *ecsx_get_target_data(ecs_world_t *world, ecs_entity_t e, ecs_entity_t type)
{
	ecs_entity_t target = ecs_get_target(world, e, type, 0);
	if (target == 0) {
		return NULL;
	}
	void const *p = ecs_get_id(world, target, type);
	return p;
}

static void print_entity(ecs_world_t *world, ecs_entity_t e)
{
	ecs_entity_t scope = ecs_get_scope(world);
	char const *scope_name = scope ? ecs_get_name(world, scope) : "";
	char *path_str = ecs_get_fullpath(world, e);
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
	char *path_str = ecs_get_fullpath(it->world, it->entities[i]);
	char *type_str = ecs_type_str(it->world, ecs_get_type(it->world, it->entities[i]));
	ecs_dbg_2(ECS_MAGENTA "%s" ECS_NORMAL " %s [%s]\n", scope_name, path_str, type_str);
	ecs_os_free(type_str);
	ecs_os_free(path_str);
}

static ecs_entity_t get_vertex_format_entity(sg_vertex_format t)
{
	switch (t) {
	case SG_VERTEXFORMAT_FLOAT2:
		return SgFloat2;
	case SG_VERTEXFORMAT_FLOAT3:
		return SgFloat3;
	case SG_VERTEXFORMAT_FLOAT4:
		return SgFloat4;
	case SG_VERTEXFORMAT_BYTE4N:
		return SgByte4n;
	case SG_VERTEXFORMAT_UBYTE4N:
		return SgUbyte4n;
	case SG_VERTEXFORMAT_USHORT2N:
		return SgUshort2n;
	default:
		return 0;
	}
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
			SgUniformType const *type = ecsx_get_target_data(world, e, ecs_id(SgUniformType));
			ecs_assert(uniform != NULL, ECS_INTERNAL_ERROR, NULL);
			ecs_assert(type != NULL, ECS_INTERNAL_ERROR, NULL);
			descs[uniform->index].name = name;
			descs[uniform->index].array_count = uniform->array_count;
			descs[uniform->index].type = type->value;
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
			SgLocation const *loc = ecs_get(world, e, SgLocation);
			sg_vertex_attr_state *outstate = descs + loc->index;
			// TODO: This code will have to change, its too much:
			if (ecs_has(world, e, SgAttributeShapePosition)) {
				outstate[0] = sshape_position_vertex_attr_state();
			} else if (ecs_has(world, e, SgAttributeShapeNormal)) {
				outstate[0] = sshape_normal_vertex_attr_state();
			} else if (ecs_has(world, e, SgAttributeShapeTextcoord)) {
				outstate[0] = sshape_texcoord_vertex_attr_state();
			} else if (ecs_has(world, e, SgAttributeShapeColor)) {
				outstate[0] = sshape_color_vertex_attr_state();
			}

			if (ecs_has(world, e, SgAttribute)) {
				SgAttribute const *attr = ecs_get(world, e, SgAttribute);
				outstate->buffer_index = attr->buffer_index;
				outstate->offset = attr->offset;
			} else {
				SgAttribute *attr = ecs_ensure(world, e, SgAttribute);
				attr->buffer_index = outstate->buffer_index;
				attr->offset = outstate->offset;
			}

			if (ecs_has_pair(world, e, ecs_id(SgVertexFormat), EcsWildcard)) {
				SgVertexFormat const *format = ecsx_get_target_data(world, e, ecs_id(SgVertexFormat));
				ecs_assert(format != NULL, ECS_INTERNAL_ERROR, NULL);
				outstate->format = format->value;
			} else {
				ecs_entity_t f = get_vertex_format_entity(outstate->format);
				ecs_add_pair(world, e, ecs_id(SgVertexFormat), f);
			}
		}
	}
}

static void set_vertex_buffers(ecs_world_t *world, ecs_entity_t parent, sg_vertex_buffer_layout_state buffers[SG_MAX_VERTEX_BUFFERS])
{
	char buf[8];
	for (int i = 0; i < SG_MAX_VERTEX_BUFFERS; ++i) {
		snprintf(buf, sizeof(buf), "buf%i", i);
		ecs_entity_t e = ecs_lookup_child(world, parent, buf);
		if (e == 0) {
			continue;
		}
		if (ecs_has(world, e, SgVertexBufferLayoutShape)) {
			buffers[i] = sshape_vertex_buffer_layout_state();
			SgVertexBufferLayout *b = ecs_ensure(world, e, SgVertexBufferLayout);
			b->stride = buffers[i].stride;
			b->step_func = buffers[i].step_func;
			b->step_rate = buffers[i].step_rate;
		} else {
			SgVertexBufferLayout const *b = ecs_get(world, e, SgVertexBufferLayout);
			if (b == NULL) {
				continue;
			}
			buffers[i].stride = b->stride;
			buffers[i].step_func = b->step_func;
			buffers[i].step_rate = b->step_rate;
		}
	}
}

void Pip_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	// SgPipelineCreate *create = ecs_field(it, SgPipelineCreate, 1); // self
	// SgAttributes * attrs = ecs_field(it, SgAttributes, 2); // up
	ecs_entity_t entity_attrs = ecs_field_src(it, 2);
	SgShader *shader = ecs_field(it, SgShader, 3); // up
	ecs_doc_set_color(world, entity_attrs, ENTITY_COLOR);

	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		ecs_doc_set_color(world, e, ENTITY_COLOR);
		print_entity_from_it(it, i);

		SgPipeline *pip = ecs_ensure(world, e, SgPipeline);

		SgIndexType const *index_type = ecsx_get_target_data(world, e, ecs_id(SgIndexType));
		SgPrimitiveType const *primitive_type = ecsx_get_target_data(world, e, ecs_id(SgPrimitiveType));
		SgCullMode const *cull_mode = ecsx_get_target_data(world, e, ecs_id(SgCullMode));

		ecs_assert(index_type != NULL, ECS_INTERNAL_ERROR, NULL);
		ecs_assert(primitive_type != NULL, ECS_INTERNAL_ERROR, NULL);
		ecs_assert(cull_mode != NULL, ECS_INTERNAL_ERROR, NULL);

		sg_pipeline_desc desc = {
		.shader = shader->id,
		.depth = {.write_enabled = true, .compare = SG_COMPAREFUNC_LESS_EQUAL},
		.index_type = index_type->value,
		.primitive_type = primitive_type->value,
		.cull_mode = cull_mode->value,
		// TODO: Use 1 when for offscreen rendering
		// Use 4 when render directly
		.sample_count = 1,
		};
		iterate_vertex_attrs(world, entity_attrs, desc.layout.attrs);
		set_vertex_buffers(world, e, desc.layout.buffers);

		pip->id = sg_make_pipeline(&desc);
	}
}

// https://github.com/SanderMertens/flecs/blob/ca73ed213310f2ca23f2afde38f72af793091e50/examples/c/entities/hierarchy/src/main.c#L52
void Shader_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	SgShaderCreate *create = ecs_field(it, SgShaderCreate, 1);
	// SgAttributes * attrs = ecs_field(it, SgAttributes, 2);
	// SgUniformBlocks * blocks = ecs_field(it, SgUniformBlocks, 3);
	// int self1 = ecs_field_is_self(it, 1);
	ecs_entity_t entity_attrs = ecs_field_src(it, 2);
	ecs_entity_t entity_blocks = ecs_field_src(it, 3);
	ecs_doc_set_color(world, entity_attrs, ENTITY_COLOR);
	ecs_doc_set_color(world, entity_blocks, ENTITY_COLOR);

	for (int i = 0; i < it->count; ++i) {
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
		iterate_shader_attrs(world, entity_attrs, desc.attrs);
		iterate_shader_blocks(world, entity_blocks, desc.vs.uniform_blocks);

		sg_shader shd = sg_make_shader(&desc);
		SgShader *shader = ecs_ensure(world, e, SgShader);
		shader->id = shd;
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
	ECS_COMPONENT_DEFINE(world, SgAttribute);
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

	ecs_add_id(world, ecs_id(SgVertexFormat), EcsUnion);
	ecs_add_id(world, ecs_id(SgIndexType), EcsUnion);
	ecs_add_id(world, ecs_id(SgPrimitiveType), EcsUnion);
	ecs_add_id(world, ecs_id(SgCullMode), EcsUnion);

	ECS_TAG_DEFINE(world, SgNone);
	ECS_TAG_DEFINE(world, SgPoints);
	ECS_TAG_DEFINE(world, SgLines);
	ECS_TAG_DEFINE(world, SgTriangles);
	ECS_TAG_DEFINE(world, SgFloat2);
	ECS_TAG_DEFINE(world, SgFloat3);
	ECS_TAG_DEFINE(world, SgFloat4);
	ECS_TAG_DEFINE(world, SgByte4n);
	ECS_TAG_DEFINE(world, SgUbyte4n);
	ECS_TAG_DEFINE(world, SgUshort2n);
	ECS_TAG_DEFINE(world, SgU16);
	ECS_TAG_DEFINE(world, SgU32);
	ECS_TAG_DEFINE(world, SgFront);
	ECS_TAG_DEFINE(world, SgBack);

	ECS_TAG_DEFINE(world, SgAttributeShapePosition);
	ECS_TAG_DEFINE(world, SgAttributeShapeNormal);
	ECS_TAG_DEFINE(world, SgAttributeShapeTextcoord);
	ECS_TAG_DEFINE(world, SgAttributeShapeColor);
	ECS_TAG_DEFINE(world, SgVertexBufferLayoutShape);

	ecs_set(world, SgPoints, SgPrimitiveType, {SG_PRIMITIVETYPE_POINTS});
	ecs_set(world, SgLines, SgPrimitiveType, {SG_PRIMITIVETYPE_LINES});
	ecs_set(world, SgTriangles, SgPrimitiveType, {SG_PRIMITIVETYPE_TRIANGLES});

	ecs_set(world, SgFloat2, SgVertexFormat, {SG_VERTEXFORMAT_FLOAT2});
	ecs_set(world, SgFloat3, SgVertexFormat, {SG_VERTEXFORMAT_FLOAT3});
	ecs_set(world, SgFloat4, SgVertexFormat, {SG_VERTEXFORMAT_FLOAT4});
	ecs_set(world, SgUbyte4n, SgVertexFormat, {SG_VERTEXFORMAT_UBYTE4N});

	ecs_set(world, SgFloat4, SgUniformType, {SG_UNIFORMTYPE_FLOAT4});

	ecs_set(world, SgNone, SgIndexType, {SG_INDEXTYPE_NONE});
	ecs_set(world, SgU16, SgIndexType, {SG_INDEXTYPE_UINT16});
	ecs_set(world, SgU32, SgIndexType, {SG_INDEXTYPE_UINT32});

	ecs_set(world, SgNone, SgCullMode, {SG_CULLMODE_NONE});
	ecs_set(world, SgFront, SgCullMode, {SG_CULLMODE_FRONT});
	ecs_set(world, SgBack, SgCullMode, {SG_CULLMODE_BACK});

	ecs_struct(world,
	{.entity = ecs_id(SgPipelineCreate),
	.members = {
	{.name = "dummy", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgShaderCreate),
	.members = {
	{.name = "filename_vs", .type = ecs_id(ecs_string_t)},
	{.name = "filename_fs", .type = ecs_id(ecs_string_t)},
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
	{.entity = ecs_id(SgAttribute),
	.members = {
	{.name = "offset", .type = ecs_id(ecs_i32_t)},
	{.name = "buffer_index", .type = ecs_id(ecs_i32_t)},
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
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgVertexFormat),
	.members = {
	{.name = "value", .type = ecs_id(ecs_i32_t)},
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
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Pip_Create,
	.query.filter.terms =
	{
	{.id = ecs_id(SgPipelineCreate), .src.flags = EcsSelf},
	{.id = ecs_id(SgAttributes), .src.trav = EcsIsA, .src.flags = EcsUp},
	{.id = ecs_id(SgShader), .src.trav = EcsIsA, .src.flags = EcsUp},
	{.id = ecs_id(SgPipeline), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Shader_Create,
	.query.filter.terms =
	{
	{.id = ecs_id(SgShaderCreate), .src.flags = EcsSelf},
	{.id = ecs_id(SgAttributes), .src.trav = EcsIsA, .src.flags = EcsUp},
	{.id = ecs_id(SgUniformBlocks), .src.trav = EcsIsA, .src.flags = EcsUp},
	{.id = ecs_id(SgShader), .oper = EcsNot}, // Adds this
	}});
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