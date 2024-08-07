#include "egsokol.h"
#include <sokol_shape.h>
#include <stdio.h>
#include <stdlib.h>
#include <egbase.h>

ECS_COMPONENT_DECLARE(SgFilter);
ECS_COMPONENT_DECLARE(SgSamplerCreate);
ECS_COMPONENT_DECLARE(SgSampler);
ECS_COMPONENT_DECLARE(SgImageCreate);
ECS_COMPONENT_DECLARE(SgImage);
ECS_COMPONENT_DECLARE(SgPipelineCreate);
ECS_COMPONENT_DECLARE(SgPipeline);
ECS_COMPONENT_DECLARE(SgShaderCreate);
ECS_COMPONENT_DECLARE(SgShader);
ECS_COMPONENT_DECLARE(SgLocation);
ECS_COMPONENT_DECLARE(SgVertexBufferLayout);
ECS_COMPONENT_DECLARE(SgAttributes);
ECS_COMPONENT_DECLARE(SgUniformBlocks);
ECS_COMPONENT_DECLARE(SgVertexFormat);
ECS_COMPONENT_DECLARE(SgPixelFormat);
ECS_COMPONENT_DECLARE(SgUniformType);
ECS_COMPONENT_DECLARE(SgIndexType);
ECS_COMPONENT_DECLARE(SgPrimitiveType);
ECS_COMPONENT_DECLARE(SgCullMode);
ECS_COMPONENT_DECLARE(SgUniformBlock);
ECS_COMPONENT_DECLARE(SgUniform);

#define ENTITY_COLOR_OK "#62fc03"
#define ENTITY_COLOR_ERR "#fc4a03"

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
			// ecs_doc_set_color(world, e, ENTITY_COLOR);
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
			// ecs_doc_set_color(world, e, ENTITY_COLOR);
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
			// ecs_doc_set_color(world, e, ENTITY_COLOR);
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
			// ecs_doc_set_color(world, e, ENTITY_COLOR);
			SgLocation *loc = ecs_get_mut(world, e, SgLocation);
			sg_vertex_attr_state *outstate = descs + loc->index;
			outstate->offset = loc->offset;
			outstate->buffer_index = loc->buffer_index;
			outstate->format = (sg_vertex_format)loc->format;
		}
	}
}

static void Pip_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	SgPipelineCreate *create = ecs_field(it, SgPipelineCreate, 0); // self
	SgShader *shader = ecs_field(it, SgShader, 1);                 // shared
	SgShaderCreate *sinfo = ecs_field(it, SgShaderCreate, 2);      // shared
	char const *statustext = "";
	char const *color = "";

	ecs_log_set_level(1);
	ecs_dbg("System Pip_Create() count:%i", it->count);
	ecs_log_push_1();
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_dbg("Entity: %s", ecs_get_name(world, e));
		ecs_log_push_1();
		sg_pipeline_desc desc = {
		.shader = (sg_shader){shader->id},
		.depth = {.write_enabled = true, .compare = SG_COMPAREFUNC_LESS_EQUAL},
		.index_type = (sg_index_type)create->indextype,
		.primitive_type = (sg_primitive_type)create->primtype,
		.cull_mode = (sg_cull_mode)create->cullmode,
		.layout.buffers = {
		// TODO: Change to array when flecs explorer array gui is nice
		{create->buflayout0.stride, create->buflayout0.step_func, create->buflayout0.step_rate},
		{create->buflayout1.stride, create->buflayout1.step_func, create->buflayout1.step_rate},
		{create->buflayout2.stride, create->buflayout2.step_func, create->buflayout2.step_rate},
		{create->buflayout3.stride, create->buflayout3.step_func, create->buflayout3.step_rate},
		{create->buflayout4.stride, create->buflayout4.step_func, create->buflayout4.step_rate},
		{create->buflayout5.stride, create->buflayout5.step_func, create->buflayout5.step_rate},
		{create->buflayout6.stride, create->buflayout6.step_func, create->buflayout6.step_rate},
		{create->buflayout7.stride, create->buflayout7.step_func, create->buflayout7.step_rate},
		},
		// TODO: Use 1 when for offscreen rendering
		// Use 4 when render directly
		.sample_count = 1,
		};
		iterate_vertex_attrs(world, sinfo->attrs, desc.layout.attrs);

		sg_pipeline pip = sg_make_pipeline(&desc);
		sg_pipeline_info info = sg_query_pipeline_info(pip);
		if (info.slot.state != SG_RESOURCESTATE_VALID) {
			ecs_warn("sg_make_pipeline failed");
			sg_destroy_pipeline(pip);
			ecs_enable(world, e, false);
			statustext = "ERR";
			color = ENTITY_COLOR_ERR;
			goto for_continue;
		}
		statustext = "OK";
		color = ENTITY_COLOR_OK;
		ecs_set(world, e, SgPipeline, {pip.id});
		ecs_dbg("sg_make_pipeline -> %i", pip.id);
		ecs_log_push_1();
		ecs_dbg("buflayout0: %i %i %i", create->buflayout0.stride, create->buflayout0.step_func, create->buflayout0.step_rate);
		ecs_dbg("buflayout1: %i %i %i", create->buflayout1.stride, create->buflayout1.step_func, create->buflayout1.step_rate);
		ecs_log_pop_1();

	for_continue: {
		char docname[128];
		snprintf(docname, 128, "%s [%s]", ecs_get_name(world, e), statustext);
		ecs_doc_set_name(world, e, docname);
		ecs_doc_set_color(world, e, color);
		ecs_log_pop_1();
	}
	}
	ecs_log_pop_1();
	ecs_log_set_level(0);
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
	char const *statustext = "";
	char const *color = "";

	ecs_log_set_level(1);
	ecs_dbg("System Shader_Create() count:%i", it->count);
	ecs_log_push_1();
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_dbg("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_1();

		sg_shader_desc desc = {0};
		ecs_dbg("eg_fs_readfile '%s'", create->filename_vs);
		desc.vs.source = eg_fs_readfile(create->filename_vs);
		if (desc.vs.source == NULL) {
			ecs_enable(world, e, false);
			ecs_warn("eg_fs_readfile '%s' failed", create->filename_vs);
			statustext = "ERR";
			color = ENTITY_COLOR_ERR;
			goto for_continue;
		}

		ecs_dbg("eg_fs_readfile '%s'", create->filename_fs);
		desc.fs.source = eg_fs_readfile(create->filename_fs);
		if (desc.fs.source == NULL) {
			ecs_enable(world, e, false);
			ecs_warn("eg_fs_readfile '%s' failed", create->filename_fs);
			statustext = "ERR";
			color = ENTITY_COLOR_ERR;
			goto for_continue;
		}

		desc.vs.entry = "main";
		desc.fs.entry = "main";

		iterate_shader_attrs(world, create->attrs, desc.attrs);
		iterate_shader_blocks(world, create->ubs, desc.vs.uniform_blocks);

		ecs_assert(desc.vs.source != NULL, ECS_INTERNAL_ERROR, NULL);
		ecs_assert(desc.fs.source != NULL, ECS_INTERNAL_ERROR, NULL);
		sg_shader shd = sg_make_shader(&desc);
		sg_shader_info info = sg_query_shader_info(shd);
		if (info.slot.state != SG_RESOURCESTATE_VALID) {
			ecs_warn("sg_make_shader() failed");
			sg_destroy_shader(shd);
			ecs_enable(world, e, false);
			statustext = "ERR";
			color = ENTITY_COLOR_ERR;
			goto for_continue;
		}

		statustext = "OK";
		color = ENTITY_COLOR_OK;
		SgShader *shader = ecs_ensure(world, e, SgShader);
		shader->id = shd.id;
		ecs_dbg("sg_make_shader() -> %i", shd.id);
	for_continue: {
		char docname[128];
		snprintf(docname, 128, "%s [%s]", ecs_get_name(world, e), statustext);
		ecs_doc_set_name(world, e, docname);
		ecs_doc_set_color(world, e, color);
		ecs_log_pop_1();
	}
	} // END FOR LOOP
	ecs_log_pop_1();
	ecs_log_set_level(0);
}

static void generate_dummy_image(void *data, int w, int h, int bytes_per_pixel)
{
	uint8_t *color = data;
	for (uint32_t x = 0; x < w; ++x) {
		for (uint32_t y = 0; y < h; ++y) {
			switch (bytes_per_pixel) {
			case 4:
				color[3] = 0xFF;
			case 3:
				color[2] = x + y;
			case 2:
				color[1] = x ^ y;
			case 1:
				color[0] = x * y;
			case 0:
				break;
			}
			color += bytes_per_pixel;
		}
	}
}

static void Img_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	SgImageCreate *create = ecs_field(it, SgImageCreate, 0);
	char const *statustext = "";
	char const *color = "";

	ecs_log_set_level(1);
	ecs_dbg("System Img_Create() count:%i", it->count);
	ecs_log_push_1();
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_dbg("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_1();

		sg_image img = sg_make_image(&(sg_image_desc){
		.usage = SG_USAGE_STREAM,
		.type = SG_IMAGETYPE_ARRAY,
		.width = create->width,
		.height = create->height,
		.num_slices = create->slices,
		.pixel_format = create->format,
		.data.subimage[0][0].ptr = 0,
		.data.subimage[0][0].size = 0,
		//.data.subimage[0][0] = SG_RANGE(pixels),
		.label = "array-texture"});
		ecs_dbg("sg_make_image() -> %i", img.id);

		{
			sg_pixelformat_info info = sg_query_pixelformat(create->format);
			int size = create->width * create->height * info.bytes_per_pixel;
			void *data = ecs_os_calloc(size);
			generate_dummy_image(data, create->width, create->height, info.bytes_per_pixel);
			sg_update_image(img,
			&(sg_image_data){
			.subimage[0][0].ptr = data,
			.subimage[0][0].size = size});
			ecs_os_free(data);
		}

		statustext = "OK";
		color = ENTITY_COLOR_OK;
		SgImage *shader = ecs_ensure(world, e, SgImage);
		shader->id = img.id;
	for_continue: {
		char docname[128];
		snprintf(docname, 128, "%s [%s]", ecs_get_name(world, e), statustext);
		ecs_doc_set_name(world, e, docname);
		ecs_doc_set_color(world, e, color);
		ecs_log_pop_1();
	}
	} // END FOR LOOP
	ecs_log_pop_1();
	ecs_log_set_level(0);
}

static void Sampler_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	SgSamplerCreate *create = ecs_field(it, SgSamplerCreate, 0);
	char const *statustext = "";
	char const *color = "";

	ecs_log_set_level(1);
	ecs_dbg("System Sampler_Create() count:%i", it->count);
	ecs_log_push_1();
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_dbg("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_1();

		sg_sampler smp = sg_make_sampler(&(sg_sampler_desc){
		.min_filter = SG_FILTER_LINEAR,
		.mag_filter = SG_FILTER_LINEAR,
		});
		ecs_dbg("sg_make_sampler() -> %i", smp.id);

		statustext = "OK";
		color = ENTITY_COLOR_OK;
		SgSampler *sampler = ecs_ensure(world, e, SgSampler);
		sampler->id = smp.id;
	for_continue: {
		char docname[128];
		snprintf(docname, 128, "%s [%s]", ecs_get_name(world, e), statustext);
		ecs_doc_set_name(world, e, docname);
		ecs_doc_set_color(world, e, color);
		ecs_log_pop_1();
	}
	} // END FOR LOOP
	ecs_log_pop_1();
	ecs_log_set_level(0);
}

void SgImport(ecs_world_t *world)
{
	ECS_MODULE(world, Sg);
	ECS_IMPORT(world, EgBase);

	ecs_set_name_prefix(world, "Sg");
	ECS_COMPONENT_DEFINE(world, SgSamplerCreate);
	ECS_COMPONENT_DEFINE(world, SgSampler);
	ECS_COMPONENT_DEFINE(world, SgFilter);
	ECS_COMPONENT_DEFINE(world, SgImageCreate);
	ECS_COMPONENT_DEFINE(world, SgImage);
	ECS_COMPONENT_DEFINE(world, SgPipelineCreate);
	ECS_COMPONENT_DEFINE(world, SgPipeline);
	ECS_COMPONENT_DEFINE(world, SgShaderCreate);
	ECS_COMPONENT_DEFINE(world, SgShader);
	ECS_COMPONENT_DEFINE(world, SgLocation);
	ECS_COMPONENT_DEFINE(world, SgVertexBufferLayout);
	ECS_COMPONENT_DEFINE(world, SgAttributes);
	ECS_COMPONENT_DEFINE(world, SgUniformBlocks);
	ECS_COMPONENT_DEFINE(world, SgVertexFormat);
	ECS_COMPONENT_DEFINE(world, SgPixelFormat);
	ECS_COMPONENT_DEFINE(world, SgUniformType);
	ECS_COMPONENT_DEFINE(world, SgIndexType);
	ECS_COMPONENT_DEFINE(world, SgPrimitiveType);
	ECS_COMPONENT_DEFINE(world, SgCullMode);
	ECS_COMPONENT_DEFINE(world, SgUniformBlock);
	ECS_COMPONENT_DEFINE(world, SgUniform);

	// clang-format off
	ecs_enum(world, {.entity = ecs_id(SgPrimitiveType), .constants = {
		{.name = "DEFAULT"       ,.value =_SG_PRIMITIVETYPE_DEFAULT      },
		{.name = "POINTS"        ,.value =SG_PRIMITIVETYPE_POINTS        },
		{.name = "LINES"         ,.value =SG_PRIMITIVETYPE_LINES         },
		{.name = "LINE_STRIP"    ,.value =SG_PRIMITIVETYPE_LINE_STRIP    },
		{.name = "TRIANGLES"     ,.value =SG_PRIMITIVETYPE_TRIANGLES     },
		{.name = "TRIANGLE_STRIP",.value =SG_PRIMITIVETYPE_TRIANGLE_STRIP},
		{.name = "NUM"           ,.value =_SG_PRIMITIVETYPE_NUM          },
		{.name = "FORCE_U32"     ,.value =_SG_PRIMITIVETYPE_FORCE_U32    },
		}});

	ecs_enum(world, {.entity = ecs_id(SgCullMode), .constants = {
		{.name = "DEFAULT"  ,.value =_SG_CULLMODE_DEFAULT  },
		{.name = "NONE"     ,.value =SG_CULLMODE_NONE      },
		{.name = "FRONT"    ,.value =SG_CULLMODE_FRONT     },
		{.name = "BACK"     ,.value =SG_CULLMODE_BACK      },
		{.name = "NUM"      ,.value =_SG_CULLMODE_NUM      },
		{.name = "FORCE_U32",.value =_SG_CULLMODE_FORCE_U32},
		}});


	ecs_enum(world, {.entity = ecs_id(SgIndexType), .constants = {
		{.name = "DEFAULT"  ,.value = _SG_INDEXTYPE_DEFAULT  },
		{.name = "NONE"     ,.value = SG_INDEXTYPE_NONE      },
		{.name = "UINT16"   ,.value = SG_INDEXTYPE_UINT16    },
		{.name = "UINT32"   ,.value = SG_INDEXTYPE_UINT32    },
		{.name = "NUM"      ,.value = _SG_INDEXTYPE_NUM      },
		{.name = "FORCE_U32",.value = _SG_INDEXTYPE_FORCE_U32},
		}});


	ecs_enum(world, {.entity = ecs_id(SgVertexFormat), .constants = {
		{.name = "INVALID"  ,.value = SG_VERTEXFORMAT_INVALID   },
		{.name = "FLOAT"    ,.value = SG_VERTEXFORMAT_FLOAT     },
		{.name = "FLOAT2"   ,.value = SG_VERTEXFORMAT_FLOAT2    },
		{.name = "FLOAT3"   ,.value = SG_VERTEXFORMAT_FLOAT3    },
		{.name = "FLOAT4"   ,.value = SG_VERTEXFORMAT_FLOAT4    },
		{.name = "BYTE4"    ,.value = SG_VERTEXFORMAT_BYTE4     },
		{.name = "BYTE4N"   ,.value = SG_VERTEXFORMAT_BYTE4N    },
		{.name = "UBYTE4"   ,.value = SG_VERTEXFORMAT_UBYTE4    },
		{.name = "UBYTE4N"  ,.value = SG_VERTEXFORMAT_UBYTE4N   },
		{.name = "SHORT2"   ,.value = SG_VERTEXFORMAT_SHORT2    },
		{.name = "SHORT2N"  ,.value = SG_VERTEXFORMAT_SHORT2N   },
		{.name = "USHORT2N" ,.value = SG_VERTEXFORMAT_USHORT2N  },
		{.name = "SHORT4"   ,.value = SG_VERTEXFORMAT_SHORT4    },
		{.name = "SHORT4N"  ,.value = SG_VERTEXFORMAT_SHORT4N   },
		{.name = "USHORT4N" ,.value = SG_VERTEXFORMAT_USHORT4N  },
		{.name = "UINT10_N2",.value = SG_VERTEXFORMAT_UINT10_N2 },
		{.name = "HALF2"    ,.value = SG_VERTEXFORMAT_HALF2     },
		{.name = "HALF4"    ,.value = SG_VERTEXFORMAT_HALF4     },
		{.name = "NUM"      ,.value = _SG_VERTEXFORMAT_NUM      },
		{.name = "FORCE_U32",.value = _SG_VERTEXFORMAT_FORCE_U32},
		}});

	ecs_enum(world, {.entity = ecs_id(SgPixelFormat), .constants = {
		{.name = "DEFAULT"        , .value = _SG_PIXELFORMAT_DEFAULT       },
		{.name = "NONE"           , .value = SG_PIXELFORMAT_NONE           },
		{.name = "R8"             , .value = SG_PIXELFORMAT_R8             },
		{.name = "R8SN"           , .value = SG_PIXELFORMAT_R8SN           },
		{.name = "R8UI"           , .value = SG_PIXELFORMAT_R8UI           },
		{.name = "R8SI"           , .value = SG_PIXELFORMAT_R8SI           },
		{.name = "R16"            , .value = SG_PIXELFORMAT_R16            },
		{.name = "R16SN"          , .value = SG_PIXELFORMAT_R16SN          },
		{.name = "R16UI"          , .value = SG_PIXELFORMAT_R16UI          },
		{.name = "R16SI"          , .value = SG_PIXELFORMAT_R16SI          },
		{.name = "R16F"           , .value = SG_PIXELFORMAT_R16F           },
		{.name = "RG8"            , .value = SG_PIXELFORMAT_RG8            },
		{.name = "RG8SN"          , .value = SG_PIXELFORMAT_RG8SN          },
		{.name = "RG8UI"          , .value = SG_PIXELFORMAT_RG8UI          },
		{.name = "RG8SI"          , .value = SG_PIXELFORMAT_RG8SI          },
		{.name = "R32UI"          , .value = SG_PIXELFORMAT_R32UI          },
		{.name = "R32SI"          , .value = SG_PIXELFORMAT_R32SI          },
		{.name = "R32F"           , .value = SG_PIXELFORMAT_R32F           },
		{.name = "RG16"           , .value = SG_PIXELFORMAT_RG16           },
		{.name = "RG16SN"         , .value = SG_PIXELFORMAT_RG16SN         },
		{.name = "RG16UI"         , .value = SG_PIXELFORMAT_RG16UI         },
		{.name = "RG16SI"         , .value = SG_PIXELFORMAT_RG16SI         },
		{.name = "RG16F"          , .value = SG_PIXELFORMAT_RG16F          },
		{.name = "RGBA8"          , .value = SG_PIXELFORMAT_RGBA8          },
		{.name = "SRGB8A8"        , .value = SG_PIXELFORMAT_SRGB8A8        },
		{.name = "RGBA8SN"        , .value = SG_PIXELFORMAT_RGBA8SN        },
		{.name = "RGBA8UI"        , .value = SG_PIXELFORMAT_RGBA8UI        },
		{.name = "RGBA8SI"        , .value = SG_PIXELFORMAT_RGBA8SI        },
		{.name = "BGRA8"          , .value = SG_PIXELFORMAT_BGRA8          },
		{.name = "RGB10A2"        , .value = SG_PIXELFORMAT_RGB10A2        },
		{.name = "RG11B10F"       , .value = SG_PIXELFORMAT_RG11B10F       },
		{.name = "RGB9E5"         , .value = SG_PIXELFORMAT_RGB9E5         },
		{.name = "RG32UI"         , .value = SG_PIXELFORMAT_RG32UI         },
		{.name = "RG32SI"         , .value = SG_PIXELFORMAT_RG32SI         },
		{.name = "RG32F"          , .value = SG_PIXELFORMAT_RG32F          },
		{.name = "RGBA16"         , .value = SG_PIXELFORMAT_RGBA16         },
		{.name = "RGBA16SN"       , .value = SG_PIXELFORMAT_RGBA16SN       },
		{.name = "RGBA16UI"       , .value = SG_PIXELFORMAT_RGBA16UI       },
		{.name = "RGBA16SI"       , .value = SG_PIXELFORMAT_RGBA16SI       },
		{.name = "RGBA16F"        , .value = SG_PIXELFORMAT_RGBA16F        },
		{.name = "RGBA32UI"       , .value = SG_PIXELFORMAT_RGBA32UI       },
		{.name = "RGBA32SI"       , .value = SG_PIXELFORMAT_RGBA32SI       },
		{.name = "RGBA32F"        , .value = SG_PIXELFORMAT_RGBA32F        },
		{.name = "DEPTH"          , .value = SG_PIXELFORMAT_DEPTH          },
		{.name = "DEPTH_STENCIL"  , .value = SG_PIXELFORMAT_DEPTH_STENCIL  },
		{.name = "BC1_RGBA"       , .value = SG_PIXELFORMAT_BC1_RGBA       },
		{.name = "BC2_RGBA"       , .value = SG_PIXELFORMAT_BC2_RGBA       },
		{.name = "BC3_RGBA"       , .value = SG_PIXELFORMAT_BC3_RGBA       },
		{.name = "BC3_SRGBA"      , .value = SG_PIXELFORMAT_BC3_SRGBA      },
		{.name = "BC4_R"          , .value = SG_PIXELFORMAT_BC4_R          },
		{.name = "BC4_RSN"        , .value = SG_PIXELFORMAT_BC4_RSN        },
		{.name = "BC5_RG"         , .value = SG_PIXELFORMAT_BC5_RG         },
		{.name = "BC5_RGSN"       , .value = SG_PIXELFORMAT_BC5_RGSN       },
		{.name = "BC6H_RGBF"      , .value = SG_PIXELFORMAT_BC6H_RGBF      },
		{.name = "BC6H_RGBUF"     , .value = SG_PIXELFORMAT_BC6H_RGBUF     },
		{.name = "BC7_RGBA"       , .value = SG_PIXELFORMAT_BC7_RGBA       },
		{.name = "BC7_SRGBA"      , .value = SG_PIXELFORMAT_BC7_SRGBA      },
		{.name = "PVRTC_RGB_2BPP" , .value = SG_PIXELFORMAT_PVRTC_RGB_2BPP },
		{.name = "PVRTC_RGB_4BPP" , .value = SG_PIXELFORMAT_PVRTC_RGB_4BPP },
		{.name = "PVRTC_RGBA_2BPP", .value = SG_PIXELFORMAT_PVRTC_RGBA_2BPP},
		{.name = "PVRTC_RGBA_4BPP", .value = SG_PIXELFORMAT_PVRTC_RGBA_4BPP},
		{.name = "ETC2_RGB8"      , .value = SG_PIXELFORMAT_ETC2_RGB8      },
		{.name = "ETC2_SRGB8"     , .value = SG_PIXELFORMAT_ETC2_SRGB8     },
		{.name = "ETC2_RGB8A1"    , .value = SG_PIXELFORMAT_ETC2_RGB8A1    },
		{.name = "ETC2_RGBA8"     , .value = SG_PIXELFORMAT_ETC2_RGBA8     },
		{.name = "ETC2_SRGB8A8"   , .value = SG_PIXELFORMAT_ETC2_SRGB8A8   },
		{.name = "ETC2_RG11"      , .value = SG_PIXELFORMAT_ETC2_RG11      },
		{.name = "ETC2_RG11SN"    , .value = SG_PIXELFORMAT_ETC2_RG11SN    },
		{.name = "ASTC_4x4_RGBA"  , .value = SG_PIXELFORMAT_ASTC_4x4_RGBA  },
		{.name = "ASTC_4x4_SRGBA" , .value = SG_PIXELFORMAT_ASTC_4x4_SRGBA },
		{.name = "NUM"            , .value = _SG_PIXELFORMAT_NUM           },
		{.name = "FORCE_U3"       , .value = _SG_PIXELFORMAT_FORCE_U32     },
		}});



	ecs_enum(world, {.entity = ecs_id(SgUniformType), .constants = {
		{.name = "INVALID"   , .value = SG_UNIFORMTYPE_INVALID   },
		{.name = "FLOAT"     , .value = SG_UNIFORMTYPE_FLOAT     },
		{.name = "FLOAT2"    , .value = SG_UNIFORMTYPE_FLOAT2    },
		{.name = "FLOAT3"    , .value = SG_UNIFORMTYPE_FLOAT3    },
		{.name = "FLOAT4"    , .value = SG_UNIFORMTYPE_FLOAT4    },
		{.name = "INT"       , .value = SG_UNIFORMTYPE_INT       },
		{.name = "INT2"      , .value = SG_UNIFORMTYPE_INT2      },
		{.name = "INT3"      , .value = SG_UNIFORMTYPE_INT3      },
		{.name = "INT4"      , .value = SG_UNIFORMTYPE_INT4      },
		{.name = "MAT4"      , .value = SG_UNIFORMTYPE_MAT4      },
		{.name = "NUM"       , .value = _SG_UNIFORMTYPE_NUM      },
		{.name = "FORCE_U32" , .value = _SG_UNIFORMTYPE_FORCE_U32},
		}});


	
	ecs_enum(world, {.entity = ecs_id(SgFilter), .constants = {
		{.name ="DEFAULT"         ,.value = _SG_FILTER_DEFAULT   },
		{.name ="NONE"            ,.value = SG_FILTER_NONE       },
		{.name ="NEAREST"         ,.value = SG_FILTER_NEAREST    },
		{.name ="LINEAR"          ,.value = SG_FILTER_LINEAR     },
		{.name ="NUM"             ,.value = _SG_FILTER_NUM       },
		{.name ="FORCE_U32"       ,.value = _SG_FILTER_FORCE_U32 },
		}});

	{
		/*
		sg_vertex_attr_state a0 = sshape_position_vertex_attr_state();
		sg_vertex_attr_state a1 = sshape_normal_vertex_attr_state();
		sg_vertex_attr_state a2 = sshape_texcoord_vertex_attr_state();
		sg_vertex_attr_state a3 = sshape_color_vertex_attr_state();
		sg_vertex_buffer_layout_state l = sshape_vertex_buffer_layout_state();
		ecs_dbg_2("");
		*/
	//sg_sampler_desc
	}

	ecs_struct(world,
	{.entity = ecs_id(SgAttributes),
	.members = {
	{.name = "count", .type = ecs_id(ecs_i32_t), .unit = EcsAmount},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgUniformBlocks),
	.members = {
	{.name = "count", .type = ecs_id(ecs_i32_t), .unit = EcsAmount},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgVertexBufferLayout),
	.members = {
	{.name = "stride", .type = ecs_id(ecs_i32_t), .unit = EcsBytes},
	{.name = "step_func", .type = ecs_id(ecs_i32_t)},
	{.name = "step_rate", .type = ecs_id(ecs_i32_t), .unit = EcsBytes},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgSamplerCreate),
	.members = {
	{.name = "min_filter", .type = ecs_id(SgFilter)},
	{.name = "mag_filter", .type = ecs_id(SgFilter)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgImageCreate),
	.members = {
	{.name = "width", .type = ecs_id(ecs_i32_t)},
	{.name = "height", .type = ecs_id(ecs_i32_t)},
	{.name = "slices", .type = ecs_id(ecs_i32_t)},
	{.name = "format", .type = ecs_id(SgPixelFormat)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgPipelineCreate),
	.members = {
	{.name = "primtype", .type = ecs_id(SgPrimitiveType)},
	{.name = "cullmode", .type = ecs_id(SgCullMode)},
	{.name = "indextype", .type = ecs_id(SgIndexType)},
	//{.name = "buflayouts", .type = ecs_id(SgVertexBufferLayout), .count = 8},
	{.name = "buflayout0", .type = ecs_id(SgVertexBufferLayout)},
	{.name = "buflayout1", .type = ecs_id(SgVertexBufferLayout)},
	{.name = "buflayout2", .type = ecs_id(SgVertexBufferLayout)},
	{.name = "buflayout3", .type = ecs_id(SgVertexBufferLayout)},
	{.name = "buflayout4", .type = ecs_id(SgVertexBufferLayout)},
	{.name = "buflayout5", .type = ecs_id(SgVertexBufferLayout)},
	{.name = "buflayout6", .type = ecs_id(SgVertexBufferLayout)},
	{.name = "buflayout7", .type = ecs_id(SgVertexBufferLayout)},
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
	{.entity = ecs_id(SgSampler),
	.members = {
	{.name = "id", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(SgImage),
	.members = {
	{.name = "id", .type = ecs_id(ecs_i32_t)},
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
	{.entity = ecs_id(SgLocation),
	.members = {
	{.name = "index", .type = ecs_id(ecs_i32_t)},
	{.name = "format", .type = ecs_id(SgVertexFormat)},
	{.name = "offset", .type = ecs_id(ecs_i32_t), .unit = EcsBytes},
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
	// clang-format on

	// clang-format off
	ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Sampler_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
		.callback = Sampler_Create,
		.query.terms =
		{
		{.id = ecs_id(SgSamplerCreate), .src.id = EcsSelf},
		{.id = ecs_id(SgSampler), .oper = EcsNot}, // Adds this
		}});

	ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Img_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
		.callback = Img_Create,
		.query.terms =
		{
		{.id = ecs_id(SgImageCreate), .src.id = EcsSelf},
		{.id = ecs_id(SgImage), .oper = EcsNot}, // Adds this
		}});

	ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Pip_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
		.callback = Pip_Create,
		.query.terms =
		{
		{.id = ecs_id(SgPipelineCreate), .src.id = EcsSelf},
		{.id = ecs_id(SgShader), .trav = EgBaseUse, .src.id = EcsUp, .inout = EcsIn},
		{.id = ecs_id(SgShaderCreate), .trav = EgBaseUse, .src.id = EcsUp, .inout = EcsIn},
		{.id = ecs_id(SgPipeline), .oper = EcsNot}, // Adds this
		}});

	ecs_system(world, {
		.entity = ecs_entity(world, {.name = "Shader_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
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