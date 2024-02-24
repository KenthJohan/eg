#include "eg/Cameras.h"
#include "eg/Components.h"
#include "eg/Spatials.h"
#include "eg/Windows.h"
#include "eg/gmath.h"
#include <assert.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif


ECS_COMPONENT_DECLARE(Camera);

void CameraUpdate(ecs_iter_t *it)
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




ECS_CTOR(Camera, ptr, {
	ptr->fov = 45;
})


void CamerasImport(ecs_world_t *world)
{
	ECS_MODULE(world, Cameras);
	ECS_IMPORT(world, Components);
	ECS_IMPORT(world, Spatials);
	ECS_IMPORT(world, Windows);

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

}