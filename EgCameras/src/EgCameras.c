#include "EgCameras.h"

#include <EgSpatials.h>
#include <EgShapes.h>
#include <EgWindows.h>
#include <egmath.h>
#include <assert.h>
#include <ecsx.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

ECS_COMPONENT_DECLARE(EgCamerasState);
ECS_COMPONENT_DECLARE(EgCamerasUnproject);

static void CameraUpdate(ecs_iter_t *it)
{
	EgCamerasState    *cam = ecs_field_self(it, EgCamerasState, 0);      // Camera state (view, projection, vp)
	Position3         *pos = ecs_field_self(it, Position3, 1);           // Position of the camera in world space
	Orientation       *o   = ecs_field_self(it, Orientation, 2);         // Orientation (quaternion) for camera rotation
	EgShapesRectangle *r   = ecs_field_shared(it, EgShapesRectangle, 3); // Window rectangle (width, height)

	for (int i = 0; i < it->count; ++i, ++cam, ++pos, ++o) {

		float rad2deg = (2.0f * M_PI) / 360.0f;
		float aspect  = r->w / r->h;
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

static void UnprojectUpdate(ecs_iter_t *it)
{
	EgCamerasUnproject   *unproj = ecs_field_self(it, EgCamerasUnproject, 0); // Currently not used, but could be used for future extensions
	Position2            *out    = ecs_field_self(it, Position2, 1);          // Outputs unprojected position in world space (XY plane)
	EgCamerasState const *cam    = ecs_field_shared(it, EgCamerasState, 2);   // Camera state (view, projection, vp)
	Position2 const      *p      = ecs_field_shared(it, Position2, 3);        // NDC Position

	(void)unproj; // Unused parameter
	for (int i = 0; i < it->count; ++i, ++out) {
		float xy[2];
		int   success = m4f32_camera_unproject_xy(&cam->vp, p->x, p->y, xy);
		if (success) {
			out->x = xy[0];
			out->y = xy[1];
		}
	}
}

ECS_CTOR(EgCamerasState, ptr, {
	ptr->fov = 45;
})

void EgCamerasImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCameras);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgWindows);
	ecs_set_name_prefix(world, "EgCameras");

	ECS_COMPONENT_DEFINE(world, EgCamerasState);
	ECS_COMPONENT_DEFINE(world, EgCamerasUnproject);

	ecs_set_hooks(world, EgCamerasState, {.ctor = ecs_ctor(EgCamerasState)});

	ecs_struct(world,
	{.entity = ecs_id(EgCamerasState),
	.members = {
	{.name = "fov", .type = ecs_id(ecs_f32_t)},
	{.name = "view", .type = ecs_id(Transformation)},
	{.name = "projection", .type = ecs_id(Transformation)},
	{.name = "vp", .type = ecs_id(Transformation)},
	}});

	ecs_system(world,
	{.entity     = ecs_entity(world, {.name = "CameraUpdate", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback    = CameraUpdate,
	.query.terms = {
	{.id = ecs_id(EgCamerasState), .src.id = EcsSelf},
	{.id = ecs_id(Position3), .src.id = EcsSelf},
	{.id = ecs_id(Orientation), .src.id = EcsSelf},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsDependsOn, .src.id = EcsUp},
	}});

	ecs_system(world,
	{.entity     = ecs_entity(world, {.name = "UnprojectUpdate", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback    = UnprojectUpdate,
	.query.terms = {
	{.id = ecs_id(EgCamerasUnproject), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsOut},
	{.id = ecs_id(EgCamerasState), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_pair(ecs_id(Position2), Normalized), .trav = EcsDependsOn, .src.id = EcsUp, .inout = EcsIn},
	}});

	/*
	ecs_system(world, {.entity = ecs_entity(world, {.name = "MouseRayCast", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = MouseRayCast,
	.query.terms =
	{
	{.id = ecs_id(Window), .src.id = ecs_id(Window)},
	{.id = ecs_id(EgCamerasState), .src.id = EcsSelf},
	{.id = ecs_id(Position3), .src.id = EcsSelf},
	{.id = ecs_id(Ray3), .src.id = EcsSelf},
	{.id = ecs_id(EgWindowsMouseRay), .src.id = EcsSelf},
	}});
	*/
}
