#include "egcameras.h"

#include <egspatials.h>
#include <egwindows.h>
#include <egmath.h>
#include <assert.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

ECS_COMPONENT_DECLARE(Camera);

static void CameraUpdate(ecs_iter_t *it)
{
	Camera *cam = ecs_field(it, Camera, 1);         // self
	Position3 *pos = ecs_field(it, Position3, 2);   // self
	Orientation *o = ecs_field(it, Orientation, 3); // self
	Window *win = ecs_field(it, Window, 4);         // singleton

	for (int i = 0; i < it->count; ++i, ++cam, ++pos, ++o) {

		float rad2deg = (2.0f * M_PI) / 360.0f;
		float aspect = win->w / win->h;
		m4f32_perspective1(&cam->projection, cam->fov * rad2deg, aspect, 0.01f, 10000.0f);

		// Apply translation (t), rotation (r), projection - which creates the view-projection-matrix (vp).
		// The view-projection-matrix can then be later used in shaders.

		m4f32 t = M4_IDENTITY;
		m4f32_translation3(&t, (float *)pos);

		m4f32 r = M4_IDENTITY;
		qf32_unit_to_m4((float *)o, &r);

		// printf("Camera:\n");
		// m4f32_print(&v);
		m4f32_mul(&cam->view, &r, &t);
		// m4f32_print(&v);

		m4f32_mul(&cam->vp, &cam->projection, &cam->view);

		// printf("Camera:\n");
		// v4f32_print(o->q);
	}
}

static void PrintMousePos(ecs_iter_t *it)
{
	Window *win = ecs_field(it, Window, 1);                          // shared
	Camera *cam = ecs_field(it, Camera, 2);                          // self
	Position3 *pos = ecs_field(it, Position3, 3);                    // self
	Ray3 *ray1 = ecs_field(it, Ray3, 4);                             // self
	EgWindowsMouseRay *winray = ecs_field(it, EgWindowsMouseRay, 5); // self
	// Orientation *rot = ecs_field(it, Orientation, 4);

	if (win->mouse_left_edge == 0) {
		return;
	}

	win->dt = it->delta_time;
	win->fps = 1.0f / it->delta_time;
	win->pos[0] = pos->x;
	win->pos[1] = pos->y;
	win->pos[2] = pos->z;

	float mouse_pos[2] = {win->canvas_mouse_x, win->canvas_mouse_y};
	float rectangle[2] = {win->canvas_width, win->canvas_height};

	/*
	sdtx_canvas(win->w/ 2.0f, win->h/ 2.0f);
	sdtx_origin(1.0f, 1.0f);
	sdtx_color3f(1.0f, 1.0f, 1.0f);
	sdtx_pos(0, 0);
	sdtx_printf("FPS: %f", 1.0f / it->delta_time);
	sdtx_pos(0, 1);
	sdtx_printf("Pos: %f %f %f", pos->x, pos->y, pos->z);
	sdtx_pos(0, 2);
	sdtx_printf("Rot: %f %f %f %f", rot->x, rot->y, rot->z, rot->w);
	sdtx_canvas(win->w / 2.0f, win->h / 2.0f);
	sdtx_origin(win->mouse_x / 16.0f, win->mouse_y / 16.0f);
	sdtx_color3f(1.0f, 1.0f, 1.0f);
	*/

	float r[4];
	r[0] = 2.0f * (mouse_pos[0] / rectangle[0]) - 1.0f;
	r[1] = 2.0f * (mouse_pos[1] / rectangle[1]) - 1.0f;
	r[1] *= -1.0f; // Why flip, hmm?
	r[2] = -1.0;
	r[3] = 1.0;

	// Eye/Camera
	// vec4 ray_eye = mat4_mul_vec4(mat4_inverse(projection), ray_clip);

	float ray_eye[4];
	m4f32 pinv;
	m4f32_inverse((float *)&cam->projection, (float *)&pinv);
	m4f32_mulv(&pinv, r, ray_eye);
	ray_eye[2] = -1.0f;
	ray_eye[3] = 0.0f;

	// Convert to world coordinates;
	// r.direction = vec3_from_vec4(mat4_mul_vec4(view, ray_eye));
	// vec3_normalize(&r.direction);

	m4f32 vinv;
	float ray_world[4];
	m4f32_inverse((float *)&cam->view, (float *)&vinv);
	m4f32_mulv(&vinv, ray_eye, ray_world);

	/*
	sdtx_pos(3, 0);
	sdtx_printf("%f %f", win->mouse_x, win->mouse_y);
	sdtx_pos(3, 1);
	sdtx_printf("%f %f", r[0], r[1]);
	sdtx_pos(3, 2);
	sdtx_printf("%f %f %f %f", ray_eye[0], ray_eye[1], ray_eye[2], ray_eye[3]);
	sdtx_pos(3, 3);
	sdtx_printf("%f %f %f %f", ray_world[0], ray_world[1], ray_world[2], ray_world[3]);
	v3f32_normalize(ray_world, ray_world);
	sdtx_pos(3, 4);
	sdtx_printf("%f %f %f %f", ray_world[0], ray_world[1], ray_world[2], ray_world[3]);
	*/
	ray1->x = ray_world[0];
	ray1->y = ray_world[1];
	ray1->z = ray_world[2];
	printf("mouse_left_edge %f %f %f\n", ray1->x, ray1->y, ray1->z);

	if (1 || win->mouse_left_edge) {
		/*
		float length = 1000.0f;
		ecs_entity_t e = ecs_lookup_fullpath(it->world, "app.line1");
		Line line = {
		    // TODO:
		    // Camera position flipped, hmm?
		    // Shoot ray from mouse position or camera position?
		    .a = {-pos->x, -pos->y, -pos->z},
		    .b = {-pos->x+ ray_world[0]*length, -pos->y+ ray_world[1]*length, -pos->z+ ray_world[2]*length}
		};
		ecs_set_ptr(it->world, e, Line, &line);
		//printf("mouse_left_edge\n");
		*/
	}
}

ECS_CTOR(Camera, ptr, {
	ptr->fov = 45;
})

void EgCamerasImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCameras);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgWindows);

	ECS_COMPONENT_DEFINE(world, Camera);

	ecs_set_hooks(world, Camera, {.ctor = ecs_ctor(Camera)});

	ecs_struct(world,
	{.entity = ecs_id(Camera),
	.members = {
	{.name = "fov", .type = ecs_id(ecs_f32_t)},
	{.name = "view", .type = ecs_id(ecs_f32_t), .count = 16},
	{.name = "projection", .type = ecs_id(ecs_f32_t), .count = 16},
	{.name = "vp", .type = ecs_id(ecs_f32_t), .count = 16},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "CameraUpdate", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = CameraUpdate,
	.query.filter.terms = {
	{.id = ecs_id(Camera), .src.flags = EcsSelf},
	{.id = ecs_id(Position3), .src.flags = EcsSelf},
	{.id = ecs_id(Orientation), .src.flags = EcsSelf},
	{.id = ecs_id(Window), .src.id = ecs_id(Window)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "PrintMousePos", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = PrintMousePos,
	.query.filter.terms =
	{
	{.id = ecs_id(Window), .src.id = ecs_id(Window)},
	{.id = ecs_id(Camera), .src.flags = EcsSelf},
	{.id = ecs_id(Position3), .src.flags = EcsSelf},
	{.id = ecs_id(Ray3), .src.flags = EcsSelf},
	{.id = ecs_id(EgWindowsMouseRay), .src.flags = EcsSelf},
	}});
}