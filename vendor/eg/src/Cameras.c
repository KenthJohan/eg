#include "eg/Cameras.h"
#include "eg/Components.h"

#include "eg/gmath.h"
#include <assert.h>

void CameraUpdate(ecs_iter_t *it)
{
	Camera *cam = ecs_field(it, Camera, 1);
	Position3 *pos = ecs_field(it, Position3, 2);
	Orientation *o = ecs_field(it, Orientation, 3);
	Window *win = ecs_field(it, Window, 4);

	for (int i = 0; i < it->count; i++) {

		float rad2deg = (2.0f * M_PI) / 360.0f;
		float aspect = win->w / win->h;
		m4f32 p; // Projection matrix
		m4f32_perspective1(&p, cam->fov * rad2deg, aspect, 0.01f, 10000.0f);

		// Apply translation (t), rotation (r), projection - which creates the view-projection-matrix (vp).
		// The view-projection-matrix can then be later used in shaders.

		m4f32 t = M4_IDENTITY;
		m4f32_translation3(&t, (float *)pos);

		m4f32 v = M4_IDENTITY;
		qf32_unit_to_m4((float*)o, &v);

		//printf("Camera:\n");
		//m4f32_print(&v);
		m4f32_mul(&v, &v, &t);
		//m4f32_print(&v);

		m4f32_mul(&cam->vp, &p, &v);

		// printf("Camera:\n");
		//v4f32_print(o->q);
	}
}

void Move(ecs_iter_t *it)
{
	Position3 *p = ecs_field(it, Position3, 1);
	Velocity3 *v = ecs_field(it, Velocity3, 2);
	Orientation *o = ecs_field(it, Orientation, 3);

	for (int i = 0; i < it->count; i++) {
		// Convert unit quaternion to rotation matrix (r)
		m4f32 r = M4_IDENTITY;
		qf32_unit_to_m4((float*)o, &r);
		// Translate postion (pos) relative to direction of camera rotation
		float dir[3];
		dir[0] = V3_DOT(r.c0, (float *)v);
		dir[1] = V3_DOT(r.c1, (float *)v);
		dir[2] = V3_DOT(r.c2, (float *)v);
		//v3f32_print((float*)dir);
		v3f32_add((float *)p, (float *)p, dir);
	}
}

void RotateQuaternion(ecs_iter_t *it)
{
	Rotate3 *rotate = ecs_field(it, Rotate3, 1);
	Orientation *orientation = ecs_field(it, Orientation, 2);
	for (int i = 0; i < it->count; i++) {
		float *look = (float *)rotate;
		float *q = orientation;
		assert(fabsf(V4_DOT(q, q) - 1.0f) < 0.1f);         // Check quaternion validity
		float dq_pitch[4];                                 // Quaternion delta pitch rotation
		float dq_yaw[4];                                   // Quaternion delta yaw rotation
		float dq_roll[4];                                  // Quaternion delta roll rotation
		qf32_normalize(q, q);                              // Normalize quaternion against floating point error
		qf32_xyza(dq_pitch, 1.0f, 0.0f, 0.0f, rotate->dx); // Make delta pitch quaternion
		qf32_xyza(dq_yaw, 0.0f, 1.0f, 0.0f, rotate->dy);   // Make delta yaw quaternion
		qf32_xyza(dq_roll, 0.0f, 0.0f, 1.0f, rotate->dz);  // Make delta roll quaternion
		qf32_mul(q, dq_roll, q);                           // Apply roll delta rotation
		qf32_mul(q, dq_yaw, q);                            // Apply yaw delta rotation
		qf32_mul(q, dq_pitch, q);                          // Apply pitch delta rotation
	}
}



void TransformationPosition(ecs_iter_t *it)
{
	Transformation *t = ecs_field(it, Transformation, 1);
	Position3 const *pos = ecs_field(it, Position3, 2);
	Orientation const *orientation = ecs_field(it, Orientation, 3);
	for (int i = 0; i < it->count; ++i, ++t, ++pos, ++orientation) {
		t->matrix = (m4f32)M4_IDENTITY;
		qf32_unit_to_m4((float*)orientation, &t->matrix);
		m4f32_translation3(&t->matrix, (float const *)pos);
	}
}



void CamerasImport(ecs_world_t *world)
{
	ECS_MODULE(world, Cameras);
	ECS_IMPORT(world, Components);

	ecs_system_init(world, &(ecs_system_desc_t){
	                           .entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	                           .callback = CameraUpdate,
	                           .query.filter.terms =
	                               {
	                                   {.id = ecs_id(Camera), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Position3), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Orientation), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Window), .src.id = ecs_id(Window)},
	                               }});

	ecs_system_init(world, &(ecs_system_desc_t){
	                           .entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	                           .callback = Move,
	                           .query.filter.terms =
	                               {
	                                   {.id = ecs_id(Position3), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Velocity3), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Orientation), .src.flags = EcsSelf}
	                               }});

	ecs_system_init(world, &(ecs_system_desc_t){
	                           .entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	                           .callback = RotateQuaternion,
	                           .query.filter.terms =
	                               {
	                                   {.id = ecs_id(Rotate3), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Orientation), .src.flags = EcsSelf},
	                               }});

	ecs_system_init(world, &(ecs_system_desc_t){
	                           .entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	                           .callback = TransformationPosition,
	                           .query.filter.terms =
	                               {
	                                   {.id = ecs_id(Transformation), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Position3), .src.flags = EcsSelf},
	                                   {.id = ecs_id(Orientation), .src.flags = EcsSelf},
	                               }});

	//ECS_SYSTEM(world, CameraUpdate, EcsOnUpdate, Camera, Position3, Orientation, Window($));
	//ECS_SYSTEM(world, Move, EcsOnUpdate, Position3, Velocity3, Orientation);
	//ECS_SYSTEM(world, RotateQuaternion, EcsOnUpdate, Rotate3, Orientation);
	//ECS_SYSTEM(world, TransformationPosition, EcsOnUpdate, Transformation, Position3, Orientation);
}