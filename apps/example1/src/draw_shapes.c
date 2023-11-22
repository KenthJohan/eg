#include "draw_shapes.h"
#include <string.h>
#include "eg/eg_fs.h"

void draw_shapes_init(draw_shapes_t *state)
{

	char const *vs = eg_fs_readfile("shapes.vs.glsl");
	char const *fs = eg_fs_readfile("shapes.fs.glsl");
	assert(vs);
	assert(fs);

	static sg_shader_desc desc;
	desc.attrs[0].name = "position";
	desc.attrs[1].name = "normal";
	desc.attrs[2].name = "texcoord";
	desc.attrs[3].name = "color0";
	desc.vs.source = vs;
	desc.vs.entry = "main";
	desc.vs.uniform_blocks[0].size = 80;
	desc.vs.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140;
	desc.vs.uniform_blocks[0].uniforms[0].name = "vs_params";
	desc.vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT4;
	desc.vs.uniform_blocks[0].uniforms[0].array_count = 5;
	desc.fs.source = fs;
	desc.fs.entry = "main";
	desc.label = "shapes_shader";
	sg_shader shd = sg_make_shader(&desc);

	// shader and pipeline object
	state->pip = sg_make_pipeline(&(sg_pipeline_desc){
	    .shader = shd,
	    .layout = {
	        .buffers[0] = sshape_vertex_buffer_layout_state(),
	        .attrs = {
	            [0] = sshape_position_vertex_attr_state(),
	            [1] = sshape_normal_vertex_attr_state(),
	            [2] = sshape_texcoord_vertex_attr_state(),
	            [3] = sshape_color_vertex_attr_state()}},
	    .index_type = SG_INDEXTYPE_UINT16,
	    .cull_mode = SG_CULLMODE_NONE,
	    .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
	});

	// shape positions
	state->positions[BOX] = (position_t){-1.0f, 1.0f, 0.0f};
	state->positions[PLANE] = (position_t){1.0f, 1.0f, 0.0f};
	state->positions[SPHERE] = (position_t){-2.0f, -1.0f, 0.0f};
	state->positions[CYLINDER] = (position_t){2.0f, -1.0f, 0.0f};
	state->positions[TORUS] = (position_t){0.0f, -1.0f, 0.0f};

	// generate shape geometries
	sshape_vertex_t vertices[6 * 1024];
	uint16_t indices[16 * 1024];
	sshape_buffer_t buf = {
	    .vertices.buffer = SSHAPE_RANGE(vertices),
	    .indices.buffer = SSHAPE_RANGE(indices),
	};
	buf = sshape_build_box(&buf, &(sshape_box_t){
	                                 .width = 10.0f,
	                                 .height = 10.0f,
	                                 .depth = 10.0f,
	                                 .tiles = 10,
	                                 .random_colors = true,
	                             });
	state->draws[BOX] = sshape_element_range(&buf);
	buf = sshape_build_plane(&buf, &(sshape_plane_t){
	                                   .width = 1.0f,
	                                   .depth = 1.0f,
	                                   .tiles = 10,
	                                   .random_colors = true,
	                               });
	state->draws[PLANE] = sshape_element_range(&buf);
	buf = sshape_build_sphere(&buf, &(sshape_sphere_t){
	                                    .radius = 0.75f,
	                                    .slices = 36,
	                                    .stacks = 20,
	                                    .random_colors = true,
	                                });
	state->draws[SPHERE] = sshape_element_range(&buf);
	buf = sshape_build_cylinder(&buf, &(sshape_cylinder_t){
	                                      .radius = 0.5f,
	                                      .height = 1.5f,
	                                      .slices = 36,
	                                      .stacks = 10,
	                                      .random_colors = true,
	                                  });
	state->draws[CYLINDER] = sshape_element_range(&buf);
	buf = sshape_build_torus(&buf, &(sshape_torus_t){
	                                   .radius = 0.5f,
	                                   .ring_radius = 0.3f,
	                                   .rings = 36,
	                                   .sides = 18,
	                                   .random_colors = true,
	                               });
	state->draws[TORUS] = sshape_element_range(&buf);
	assert(buf.valid);

	// one vertex/index-buffer-pair for all shapes
	const sg_buffer_desc vbuf_desc = sshape_vertex_buffer_desc(&buf);
	const sg_buffer_desc ibuf_desc = sshape_index_buffer_desc(&buf);
	state->vbuf = sg_make_buffer(&vbuf_desc);
	state->ibuf = sg_make_buffer(&ibuf_desc);
}

void draw_shapes_frame(draw_shapes_t *state, m4f32 *vp)
{
	sg_apply_pipeline(state->pip);
	sg_apply_bindings(&(sg_bindings){
	    .vertex_buffers[0] = state->vbuf,
	    .index_buffer = state->ibuf});
	memcpy(state->vs_params.mvp, vp, sizeof(m4f32));
	for (int i = 0; i < NUM_SHAPES; i++) {
		m4f32 model = M4_IDENTITY;
		m4f32_translation3(&model, (float *)(state->positions + i));
		m4f32_mul((m4f32 *)state->vs_params.mvp, vp, &model);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(state->vs_params));
		sg_draw(state->draws[i].base_element, state->draws[i].num_elements, 1);
	}
}

void draw_shapes1(draw_shapes_t *state, m4f32 *vp, float a)
{
	sshape_vertex_t vertices[6 * 1024];
	uint16_t indices[16 * 1024];
	sshape_buffer_t buf = {
	    .vertices.buffer = SSHAPE_RANGE(vertices),
	    .indices.buffer = SSHAPE_RANGE(indices),
	};
	buf = sshape_build_box(&buf, &(sshape_box_t){
	                                 .width = 10.0f + a,
	                                 .height = 10.0f,
	                                 .depth = 10.0f,
	                                 .tiles = 10,
	                                 .random_colors = true,
	                             });

	sshape_element_range_t draws = sshape_element_range(&buf);
	sg_apply_pipeline(state->pip);
	sg_apply_bindings(&(sg_bindings){
	    .vertex_buffers[0] = state->vbuf,
	    .index_buffer = state->ibuf});


	memcpy(state->vs_params.mvp, vp, sizeof(m4f32));

	m4f32 model = M4_IDENTITY;
	m4f32_translation3(&model, (float *)(state->positions));
	m4f32_mul((m4f32 *)state->vs_params.mvp, vp, &model);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(state->vs_params));
	sg_draw(draws.base_element, draws.num_elements, 1);
}