#include <bake_test.h>
#include <EgBase.h>
#include <EgDraws.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <math.h>
#include <string.h>

#include "../../../src/verts.h"

static ecs_world_t *world;
static ecs_vec_t vertices;
static verts_config_t config;

void Verts_setup(void)
{
	world = ecs_init();
	ecs_vec_init_t(NULL, &vertices, EgDraws_Vertex, 0);
	config = (verts_config_t){
		.pixscale = 0.25f,
		.transform = M3_IDENTITY,
		.r = 18, .g = 52, .b = 86, .a = 0
	};
}

void Verts_teardown(void)
{
	ecs_vec_fini_t(NULL, &vertices, EgDraws_Vertex);
	ecs_fini(world);
}

static int close_to(float a, float b)
{
	return fabsf(a - b) < 0.0001f;
}

void Verts_test_attributes_and_append(void)
{
	config.transform.c2[0] = 4;
	config.transform.c2[1] = -3;
	verts_add_triangle(&vertices, &config, 0, 0, 2, 0, 0, 1);
	verts_add_rectangle(&vertices, &config, 2, 4);
	test_int(ecs_vec_count(&vertices), 9);
	EgDraws_Vertex *v = ecs_vec_first_t(&vertices, EgDraws_Vertex);
	test_assert(close_to(v[0].position[0], 4));
	test_assert(close_to(v[0].position[1], -3));
	test_assert(close_to(v[1].position[0], 6));
	test_assert(close_to(v[2].position[1], -2));
	test_assert(close_to(v[3].position[0], 3));
	test_assert(close_to(v[3].position[1], -5));
	test_assert(close_to(v[4].position[0], 5));
	test_assert(close_to(v[4].position[1], -5));
	test_assert(close_to(v[5].position[0], 5));
	test_assert(close_to(v[5].position[1], -1));
	for (int i = 0; i < 9; ++i) {
		test_assert(close_to(v[i].uv[0], 0.5f / EG_DRAWS_ATLAS_WIDTH));
		test_assert(close_to(v[i].uv[1], 0.5f / EG_DRAWS_ATLAS_HEIGHT));
		test_int(v[i].rgba[0], 18);
		test_int(v[i].rgba[1], 52);
		test_int(v[i].rgba[2], 86);
		test_int(v[i].rgba[3], 0);
	}
}

void Verts_test_shape_counts_and_degenerate_inputs(void)
{
	EgDraws_Vec2 polygon[] = {{0, 0}, {2, 0}, {2, 1}, {0, 1}};
	verts_add_line(&vertices, &config, 0, 0, 2, 0, 1);
	test_int(ecs_vec_count(&vertices), 6);
	verts_add_point(&vertices, &config, 1);
	test_int(ecs_vec_count(&vertices), 12);
	verts_add_circle(&vertices, &config, 1);
	test_int(ecs_vec_count(&vertices), 108);
	verts_add_circle_outline(&vertices, &config, 1, 1);
	test_int(ecs_vec_count(&vertices), 300);
	verts_add_capsule_outline(&vertices, &config, 0, 0, 2, 0, 1, 1);
	test_int(ecs_vec_count(&vertices), 594);
	verts_add_transform(&vertices, &config, 1);
	test_int(ecs_vec_count(&vertices), 606);
	verts_add_rectangle_outline(&vertices, &config, 2, 1, 1);
	test_int(ecs_vec_count(&vertices), 630);
	verts_add_bounds(&vertices, &config, 0, 0, 2, 1);
	test_int(ecs_vec_count(&vertices), 654);
	verts_add_polygon(&vertices, &config, polygon, 4);
	test_int(ecs_vec_count(&vertices), 660);

	verts_add_line(&vertices, &config, 0, 0, 0, 0, 1);
	verts_add_line(&vertices, &config, 0, 0, 1, 1, 0);
	verts_add_point(&vertices, &config, 0);
	verts_add_circle(&vertices, &config, -1);
	verts_add_circle_outline(&vertices, &config, 1, 0);
	verts_add_capsule_outline(&vertices, &config, 0, 0, 0, 0, 1, 1);
	verts_add_rectangle(&vertices, &config, 0, 1);
	verts_add_rectangle_outline(&vertices, &config, 1, 1, 0);
	verts_add_polygon(&vertices, &config, polygon, 2);
	test_int(ecs_vec_count(&vertices), 660);
}

void Verts_test_pixel_scale(void)
{
	verts_add_line(&vertices, &config, 0, 0, 2, 0, 2);
	EgDraws_Vertex *v = ecs_vec_first_t(&vertices, EgDraws_Vertex);
	test_assert(close_to(v[0].position[1], 0.25f));
	test_assert(close_to(v[1].position[1], -0.25f));
	ecs_vec_clear(&vertices);
	config.pixscale = 0;
	verts_add_point(&vertices, &config, 2);
	v = ecs_vec_first_t(&vertices, EgDraws_Vertex);
	test_assert(close_to(v[0].position[0], -2));
	test_assert(close_to(v[0].position[1], -2));
	config.pixscale = -2;
	verts_add_line(&vertices, &config, 0, 0, 2, 0, 2);
	v = ecs_vec_first_t(&vertices, EgDraws_Vertex);
	test_assert(close_to(v[6].position[1], 1));
}

void Verts_test_text(void)
{
	const unsigned char *bitmap = EgDraws_Font_GetBitmap();
	verts_add_text(&vertices, &config, 24, "A\tB\nC");
	if (bitmap == NULL) {
		test_int(ecs_vec_count(&vertices), 0);
		return;
	}
	test_assert(bitmap == EgDraws_Font_GetBitmap());
	test_int(ecs_vec_count(&vertices), 18);
	EgDraws_Vertex *v = ecs_vec_first_t(&vertices, EgDraws_Vertex);
	test_int(v[0].rgba[3], 0);
	test_assert(v[0].uv[0] != v[1].uv[0]);
	ecs_vec_clear(&vertices);
	verts_add_text(&vertices, &config, 24, "?");
	EgDraws_Vertex expected[6];
	memcpy(expected, ecs_vec_first(&vertices), sizeof(expected));
	ecs_vec_clear(&vertices);
	verts_add_text(&vertices, &config, 24, "\1");
	test_int(ecs_vec_count(&vertices), 6);
	test_assert(memcmp(expected, ecs_vec_first(&vertices), sizeof(expected)) == 0);
	ecs_vec_clear(&vertices);
	verts_add_text(&vertices, &config, 0, "A");
	test_int(ecs_vec_count(&vertices), 0);
}

void Verts_test_collectors(void)
{
	EgDraws_Import(world);
	ecs_entity_t root = ecs_new(world);
	ecs_add(world, root, EgDraws_List);
	EgDraws_List *list = ecs_get_mut(world, root, EgDraws_List);
	EgDraws_List_SetPixelScale(list, 0.5f);

	ecs_entity_t solid = ecs_new_w_pair(world, EcsChildOf, root);
	ecs_set(world, solid, EgDraws_Solid, {.color = 0x00123456});
	ecs_set(world, solid, EgShapesRectangle, {.w = 2, .h = 4});
	ecs_set(world, solid, WorldTransform3, {.matrix = M3_IDENTITY});
	ecs_set(world, solid, EgDraws_Z, {.z = -1});

	ecs_entity_t border = ecs_new_w_pair(world, EcsChildOf, root);
	ecs_set(world, border, EgDraws_Border, {.thickness = 2, .color = 0x7fabcdef});
	ecs_set(world, border, EgShapesRectangle, {.w = 2, .h = 4});
	ecs_set(world, border, WorldTransform3, {.matrix = M3_IDENTITY});
	ecs_set(world, border, EgDraws_Z, {.z = 2});

	ecs_entity_t text = ecs_new_w_pair(world, EcsChildOf, root);
	ecs_set(world, text, EgBaseText, {.value = "A"});
	ecs_set(world, text, EgBaseFont, {.font_size = 0, .color = 0x00112233});
	ecs_set(world, text, WorldTransform3, {.matrix = M3_IDENTITY});
	ecs_set(world, text, EgDraws_Z, {.z = 1});

	ecs_entity_t invalid = ecs_new_w_pair(world, EcsChildOf, root);
	ecs_set(world, invalid, EgDraws_Solid, {.color = 0xff000000});
	ecs_set(world, invalid, EgShapesRectangle, {.w = 0, .h = 1});
	ecs_set(world, invalid, WorldTransform3, {.matrix = M3_IDENTITY});
	ecs_set(world, invalid, EgDraws_Z, {.z = 8});

	ecs_progress(world, 0);
	list = ecs_get_mut(world, root, EgDraws_List);
	test_int(ecs_vec_count(&list->layers), 3);
	ecs_vec_t *layer0 = EgDraws_List_GetLayer(list, 0);
	ecs_vec_t *layer1 = EgDraws_List_GetLayer(list, 1);
	ecs_vec_t *layer2 = EgDraws_List_GetLayer(list, 2);
	test_int(ecs_vec_count(layer0), 6);
	test_int(ecs_vec_count(layer2), 24);
	EgDraws_Vertex *v = ecs_vec_first_t(layer0, EgDraws_Vertex);
	test_int(v[0].rgba[0], 0x12);
	test_int(v[0].rgba[1], 0x34);
	test_int(v[0].rgba[2], 0x56);
	test_int(v[0].rgba[3], 255);
	v = ecs_vec_first_t(layer2, EgDraws_Vertex);
	test_int(v[0].rgba[0], 0xab);
	test_int(v[0].rgba[3], 0x7f);
	test_assert(close_to(v[0].position[1], -1.5f));
	if (EgDraws_Font_GetBitmap() != NULL) {
		test_int(ecs_vec_count(layer1), 6);
		v = ecs_vec_first_t(layer1, EgDraws_Vertex);
		test_int(v[0].rgba[3], 255);
		config.pixscale = 0.5f;
		config.r = 0x11;
		config.g = 0x22;
		config.b = 0x33;
		config.a = 255;
		verts_add_text(&vertices, &config, 24, "A");
		test_int(ecs_vec_count(&vertices), 6);
		test_assert(memcmp(ecs_vec_first(&vertices), v, 6 * sizeof(*v)) == 0);
	}
	ecs_progress(world, 0);
	test_int(ecs_vec_count(EgDraws_List_GetLayer(list, 0)), 6);
	test_int(ecs_vec_count(EgDraws_List_GetLayer(list, 2)), 24);
}
