#include "EgSpatialsSystems.h"

#include <EgSpatials.h>
#include <ecsx.h>
#include <math.h>

static void Position3World_Reset(ecs_iter_t *it)
{
	Position3WorldOffset *l = ecs_field_self(it, Position3WorldOffset, 0); // out
	for (int i = 0; i < it->count; ++i, ++l) {
		l[0].x = 0;
		l[0].y = 0;
		l[0].z = 0;
	}
}

static void Orientation_To_RotMat3(ecs_iter_t *it)
{
	RotMat3     *r = ecs_field_self(it, RotMat3, 0);     // out
	Orientation *o = ecs_field_self(it, Orientation, 1); // in
	for (int i = 0; i < it->count; ++i, ++o, ++r) {
		qf32_unit_to_m3((float *)o, (m3f32 *)r);
	}
}

static void Orientation_Cascade(ecs_iter_t *it)
{
	OrientationWorld *g = ecs_field_self(it, OrientationWorld, 0); // out
	Orientation      *l = ecs_field_self(it, Orientation, 1);      // in
	OrientationWorld *p = ecs_field(it, OrientationWorld, 2);      // in, optional can be NULL
	for (int i = 0; i < it->count; ++i, ++l, ++g) {
		g->x = l->x;
		g->y = l->y;
		g->z = l->z;
		g->w = l->w;
		if (p) {
			qf32_mul((float *)g, (float const *)p, (float const *)g);
		}
	}
}

static void Orientation_Rotate1(ecs_iter_t *it)
{
	Orientation   *q = ecs_field_self(it, Orientation, 0); // out
	Rotate3 const *r = ecs_field_self(it, Rotate3, 1);     // in
	for (int i = 0; i < it->count; ++i, ++r, ++q) {
		// Check quaternion validity
		ecs_assert(fabsf(V4_DOT((float *)q, (float *)q) - 1.0f) < 0.1f, ECS_INTERNAL_ERROR, NULL);

		// Normalize quaternion against floating point error
		qf32_normalize((float *)q, (float *)q, 0.000001f);

		float dp[4];                            // Quaternion delta pitch rotation
		float dy[4];                            // Quaternion delta yaw rotation
		float dr[4];                            // Quaternion delta roll rotation
		qf32_xyza(dp, 1.0f, 0.0f, 0.0f, r->dx); // Make delta pitch quaternion
		qf32_xyza(dy, 0.0f, 1.0f, 0.0f, r->dy); // Make delta yaw quaternion
		qf32_xyza(dr, 0.0f, 0.0f, 1.0f, r->dz); // Make delta roll quaternion
		qf32_mul((float *)q, (float *)q, dr);   // Apply roll delta rotation
		qf32_mul((float *)q, (float *)q, dy);   // Apply yaw delta rotation
		qf32_mul((float *)q, (float *)q, dp);   // Apply pitch delta rotation
	}
}

static void Orientation_Rotate2(ecs_iter_t *it)
{
	Orientation   *q = ecs_field_self(it, Orientation, 0); // out
	Rotate3 const *r = ecs_field_self(it, Rotate3, 1);     // in
	for (int i = 0; i < it->count; ++i, ++r, ++q) {
		// Check quaternion validity
		ecs_assert(fabsf(V4_DOT((float *)q, (float *)q) - 1.0f) < 0.1f, ECS_INTERNAL_ERROR, NULL);

		// Normalize quaternion against floating point error
		qf32_normalize((float *)q, (float *)q, 0.000001f);

		float dp[4];                            // Quaternion delta pitch rotation
		float dy[4];                            // Quaternion delta yaw rotation
		float dr[4];                            // Quaternion delta roll rotation
		qf32_xyza(dp, 1.0f, 0.0f, 0.0f, r->dx); // Make delta pitch quaternion
		qf32_xyza(dy, 0.0f, 1.0f, 0.0f, r->dy); // Make delta yaw quaternion
		qf32_xyza(dr, 0.0f, 0.0f, 1.0f, r->dz); // Make delta roll quaternion
		qf32_mul((float *)q, dr, (float *)q);   // Apply roll delta rotation
		qf32_mul((float *)q, dy, (float *)q);   // Apply yaw delta rotation
		qf32_mul((float *)q, dp, (float *)q);   // Apply pitch delta rotation
	}
}

static void Matrix4_Cascade(ecs_iter_t *it)
{
	Matrix4                    *t              = ecs_field_self(it, Matrix4, 0);         // out
	Position3 const            *p              = ecs_field_self(it, Position3, 1);       // in
	Orientation const          *q              = ecs_field_self(it, Orientation, 2);     // in
	Scale3 const               *s              = ecs_field_self(it, Scale3, 3);          // in
	Position3WorldOffset const *offset         = ecs_field(it, Position3WorldOffset, 4); // in, optional effect offset
	Position3World             *world_position = ecs_field_self(it, Position3World, 5);  // out
	Matrix4 const              *parent         = ecs_field(it, Matrix4, 6);              // parent, optional
	for (int i = 0; i < it->count; ++i, ++t, ++p, ++q, ++s, ++world_position) {
		m4f32 local;
		m4f32 world;
		m4f32_trs((float const *)p, (float const *)q, (float const *)s, &local);
		if (parent) {
			m4f32_mul(&world, &parent->matrix, &local);
		} else {
			world = local;
		}

		if (offset) {
			world.c3[0] += offset[i].x;
			world.c3[1] += offset[i].y;
			world.c3[2] += offset[i].z;
		}
		world_position->x = world.c3[0];
		world_position->y = world.c3[1];
		world_position->z = world.c3[2];
		t->matrix         = world;
	}
}

static void Position3_Move(ecs_iter_t *it)
{
	Position3         *p = ecs_field_self(it, Position3, 0);   // out
	Velocity3 const   *v = ecs_field_self(it, Velocity3, 1);   // in
	Orientation const *o = ecs_field_self(it, Orientation, 2); // in

	for (int i = 0; i < it->count; ++i, ++p, ++v, ++o) {
		// Convert unit quaternion to rotation matrix (r)
		m4f32 r = M4_IDENTITY;
		qf32_unit_to_m4((float *)o, &r);

		// Translate postion (pos) relative to direction of camera rotation:
		float dir[3];
		dir[0] = V3_DOT((float *)v, r.c0);
		dir[1] = V3_DOT((float *)v, r.c1);
		dir[2] = V3_DOT((float *)v, r.c2);

		v3f32_mul((float *)dir, (float *)dir, it->delta_time);
		v3f32_add((float *)p, (float *)p, dir);
	}
}

static void SinewaveSystem(ecs_iter_t *it)
{
	Position3WorldOffset *p = ecs_field_self(it, Position3WorldOffset, 0); // out
	Sinewave const       *w = ecs_field_self(it, Sinewave, 1);             // in
	for (int i = 0; i < it->count; ++i, ++w, ++p) {
		ecs_time_t time;
		ecs_os_get_time(&time);
		double t = (float)time.sec + ((double)time.nanosec / (1000.0 * 1000.0 * 1000.0));
		double a = w->frequency * t * 2.0 * 3.14;
		p->x += sin(a) * w->amplitude;
		p->y += cos(a) * w->amplitude;
	}
}

static void EulerToQ(ecs_iter_t *it)
{
	Orientation       *o = ecs_field_self(it, Orientation, 0); // out
	EulerAngles const *e = ecs_field_self(it, EulerAngles, 1); // in
	for (int i = 0; i < it->count; ++i, ++e, ++o) {
		qf32_from_euler((float *)o, e->pitch, e->yaw, e->roll);
	}
}

void EgSpatialsSystemsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSpatialsSystems);
	ecs_set_name_prefix(world, "EgSpatialsSystems");

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Orientation_Rotate1"}),
		.phase       = EcsOnUpdate,
		.callback    = Orientation_Rotate1,
		.query.terms = {
		{.id = ecs_id(Orientation), .inout = EcsOut},
		{.id = ecs_id(Rotate3), .inout = EcsIn},
		{.id = RotateOrder1},
		}});
		ecs_doc_set_detail(world, e,
		"Applies pitch, yaw, and roll deltas to the local orientation by post-multiplying each delta. "
		"Use RotateOrder1 for this rotation order.");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Orientation_Rotate2"}),
		.phase       = EcsOnUpdate,
		.callback    = Orientation_Rotate2,
		.query.terms = {
		{.id = ecs_id(Orientation), .inout = EcsOut},
		{.id = ecs_id(Rotate3), .inout = EcsIn},
		{.id = RotateOrder2},
		}});
		ecs_doc_set_detail(world, e,
		"Applies pitch, yaw, and roll deltas to the local orientation by pre-multiplying each delta. "
		"Use RotateOrder2 for this rotation order.");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Position3World_Reset"}),
		.phase       = EcsPreUpdate,
		.callback    = Position3World_Reset,
		.query.terms = {
		{.id = ecs_id(Position3WorldOffset), .inout = EcsOut},
		}});
		ecs_doc_set_detail(world, e,
		"Clears per-frame world-position effect offsets before effect systems add their contributions.");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "EulerToQ"}),
		.phase       = EcsOnUpdate,
		.callback    = EulerToQ,
		.query.terms = {
		{.id = ecs_id(Orientation), .inout = EcsOut},
		{.id = ecs_id(EulerAngles), .inout = EcsIn},
		}});
		ecs_doc_set_detail(world, e,
		"Converts pitch, yaw, and roll Euler angles into the entity's local orientation quaternion.");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Orientation_To_RotMat3"}),
		.phase       = EcsOnUpdate,
		.callback    = Orientation_To_RotMat3,
		.query.terms = {
		{.id = ecs_id(RotMat3), .inout = EcsOut},
		{.id = ecs_id(Orientation), .inout = EcsIn},
		}});
		ecs_doc_set_detail(world, e,
		"Converts the local orientation quaternion into a 3x3 rotation matrix for systems that consume matrix-form rotation.");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Position3_Move"}),
		.phase       = EcsOnUpdate,
		.callback    = Position3_Move,
		.query.terms = {
		{.id = ecs_id(Position3), .inout = EcsOut},
		{.id = ecs_id(Velocity3), .inout = EcsIn},
		{.id = ecs_id(Orientation), .inout = EcsIn},
		}});
		ecs_doc_set_detail(world, e,
		"Moves the entity's local position using velocity rotated by its local orientation and scaled by frame delta time.");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "SinewaveSystem"}),
		.phase       = EcsOnUpdate,
		.callback    = SinewaveSystem,
		.query.terms = {
		{.id = ecs_id(Position3WorldOffset), .inout = EcsOut},
		{.id = ecs_id(Sinewave), .inout = EcsIn},
		}});
		ecs_doc_set_detail(world, e,
		"Adds a time-based sinusoidal world-position offset using the entity's Sinewave frequency and amplitude.");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Orientation_Cascade"}),
		.phase       = EcsOnUpdate,
		.callback    = Orientation_Cascade,
		.query.terms = {
		{.id = ecs_id(OrientationWorld), .inout = EcsOut},
		{.id = ecs_id(Orientation), .inout = EcsIn},
		{.id = ecs_id(OrientationWorld), .src.id = EcsCascade, .inout = EcsIn, .oper = EcsOptional},
		}});
		ecs_doc_set_detail(world, e,
		"Propagates local orientations through the entity hierarchy into OrientationWorld. "
		"Use when systems need an entity's world-space rotation as a quaternion.");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Matrix4_Cascade"}),
		.phase       = EcsPostUpdate,
		.callback    = Matrix4_Cascade,
		.query.terms = {
		{.id = ecs_id(Matrix4), .inout = EcsOut},
		{.id = ecs_id(Position3), .inout = EcsIn},
		{.id = ecs_id(Orientation), .inout = EcsIn},
		{.id = ecs_id(Scale3), .inout = EcsIn},
		{.id = ecs_id(Position3WorldOffset), .inout = EcsIn, .oper = EcsOptional},
		{.id = ecs_id(Position3World), .inout = EcsOut},
		{.id = ecs_id(Matrix4), .src.id = EcsCascade, .inout = EcsIn, .oper = EcsOptional},
		}});
		ecs_doc_set_detail(world, e,
		"Composes local position, orientation, and scale with parent matrices to produce a world matrix. "
		"Applies any Position3WorldOffset and writes the resulting world position to Position3World.");
	}
}
