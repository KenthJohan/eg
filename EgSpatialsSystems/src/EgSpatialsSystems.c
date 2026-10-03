#include "EgSpatialsSystems.h"

#include <EgSpatials.h>
#include <ecsx.h>
#include <math.h>

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

static void Transform4_Cascade(ecs_iter_t *it)
{
	Transform4            *l  = ecs_field_self(it, Transform4, 0);      // Child, in
	WorldTransform4       *w  = ecs_field_self(it, WorldTransform4, 1); // Child, out
	WorldTransform4 const *w0 = ecs_field(it, WorldTransform4, 2);      // optional parent, in

	if (w0) {
		for (int i = 0; i < it->count; ++i, ++l, ++w) {
			m4f32_mul(&w->matrix, &w0->matrix, &l->matrix);
		}
	} else {
		for (int i = 0; i < it->count; ++i, ++l, ++w) {
			w->matrix = l->matrix;
		}
	}
}

static void Transform3_Cascade(ecs_iter_t *it)
{
	Transform3            *l  = ecs_field_self(it, Transform3, 0);      // Child, in
	WorldTransform3       *w  = ecs_field_self(it, WorldTransform3, 1); // Child, out
	WorldTransform3 const *w0 = ecs_field(it, WorldTransform3, 2);      // optional parent, in

	if (w0) {
		for (int i = 0; i < it->count; ++i, ++l, ++w) {
			m3f32_mul(&w->matrix, &w0->matrix, &l->matrix);
		}
	} else {
		for (int i = 0; i < it->count; ++i, ++l, ++w) {
			w->matrix = l->matrix;
		}
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

static void EulerToQ(ecs_iter_t *it)
{
	Orientation       *o = ecs_field_self(it, Orientation, 0); // out
	EulerAngles const *e = ecs_field_self(it, EulerAngles, 1); // in
	for (int i = 0; i < it->count; ++i, ++e, ++o) {
		qf32_from_euler((float *)o, e->pitch, e->yaw, e->roll);
	}
}

static void Transform3_trs(ecs_iter_t *it)
{
	Transform3      *t = ecs_field_self(it, Transform3, 0); // out
	Position2 const *p = ecs_field_self(it, Position2, 1);  // in
	Scale2 const    *s = ecs_field_self(it, Scale2, 2);     // in
	Rotation2 const *r = ecs_field_self(it, Rotation2, 3);  // in, expressed as an angle in radians
	for (int i = 0; i < it->count; ++i, ++t, ++p, ++s, ++r) {
		// Convert 2D rotation radians to a 2D quaternion representation with cosine and sine:
		float q[2] = {cosf(r->radians * 0.5f), sinf(r->radians * 0.5f)};
		// Construct the local transform matrix from position, scale, and rotation
		m3f32_trs((float *)p, (float *)q, (float *)s, &(t->matrix));
	}
}

static void Transform4_trs(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; ++i) {
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
		.entity      = ecs_entity(world, {.name = "Transform3_trs"}),
		.phase       = EcsPostUpdate,
		.callback    = Transform3_trs,
		.query.terms = {
		{.id = ecs_id(Transform3), .inout = EcsOut},
		{.id = ecs_id(Position2), .inout = EcsIn},
		{.id = ecs_id(Scale2), .inout = EcsIn},
		{.id = ecs_id(Rotation2), .inout = EcsIn}, // Euler angles representing the entity's local rotation
		}});
		ecs_doc_set_detail(world, e, "");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Transform4_trs"}),
		.phase       = EcsPostUpdate,
		.callback    = Transform4_trs,
		.query.terms = {
		{.id = ecs_id(Transform4), .inout = EcsOut},
		{.id = ecs_id(Position3), .inout = EcsIn},
		{.id = ecs_id(Scale3), .inout = EcsIn},
		{.id = ecs_id(Orientation), .inout = EcsIn}, // Quaternion representing the entity's local rotation
		}});
		ecs_doc_set_detail(world, e, "");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Transform4_Cascade"}),
		.phase       = EcsPostUpdate,
		.callback    = Transform4_Cascade,
		.query.terms = {
		{.id = ecs_id(Transform4), .inout = EcsOut},
		{.id = ecs_id(WorldTransform4), .inout = EcsIn},
		{.id = ecs_id(WorldTransform4), .src.id = EcsCascade, .inout = EcsIn, .oper = EcsOptional},
		}});
		ecs_doc_set_detail(world, e, "");
	}

	{
		ecs_entity_t e = ecs_system_init(world,
		&(ecs_system_desc_t){
		.entity      = ecs_entity(world, {.name = "Transform3_Cascade"}),
		.phase       = EcsPostUpdate,
		.callback    = Transform3_Cascade,
		.query.terms = {
		{.id = ecs_id(Transform3), .inout = EcsOut},
		{.id = ecs_id(WorldTransform3), .inout = EcsIn},
		{.id = ecs_id(WorldTransform3), .src.id = EcsCascade, .inout = EcsIn, .oper = EcsOptional},
		}});
		ecs_doc_set_detail(world, e, "");
	}
}
