#include "eg/Spatials.h"


ECS_COMPONENT_DECLARE(Position2);
ECS_COMPONENT_DECLARE(Position3);
ECS_COMPONENT_DECLARE(Position3World);
ECS_COMPONENT_DECLARE(Velocity2);
ECS_COMPONENT_DECLARE(Velocity3);
ECS_COMPONENT_DECLARE(Rotate3);
ECS_COMPONENT_DECLARE(RotMat3);
ECS_COMPONENT_DECLARE(Orientation);
ECS_COMPONENT_DECLARE(OrientationWorld);
ECS_COMPONENT_DECLARE(EulerAngles);
ECS_COMPONENT_DECLARE(Transformation);
ECS_COMPONENT_DECLARE(Sinewave);
ECS_TAG_DECLARE(EgRotateOrder1);
ECS_TAG_DECLARE(EgRotateOrder2);
ECS_TAG_DECLARE(EgPositionWorldNoReset);


static void Position3World_Reset(ecs_iter_t *it)
{
	Position3World *l = ecs_field(it, Position3World, 1); // self, out
	for (int i = 0; i < it->count; ++i, ++l) {
		l[0].x = 0;
		l[0].y = 0;
		l[0].z = 0;
	}
}

static void Orientation_To_RotMat3(ecs_iter_t *it)
{
	Orientation *o = ecs_field(it, Orientation, 1); // self, in
	RotMat3 *r = ecs_field(it, RotMat3, 2); // self, out
	for (int i = 0; i < it->count; ++i, ++o, ++r) {
		qf32_unit_to_m3((float *)o, (m3f32*)r);
	}
}




static void Orientation_Cascade(ecs_iter_t *it)
{
	Orientation *l = ecs_field(it, Orientation, 1); // self, in
	OrientationWorld *g = ecs_field(it, OrientationWorld, 2); // self, out
	OrientationWorld *p = ecs_field(it, OrientationWorld, 3); // parent, in
	for (int i = 0; i < it->count; ++i, ++l, ++g) {
		g[0].x = l[0].x;
		g[0].y = l[0].y;
		g[0].z = l[0].z;
		g[0].w = l[0].w;
		if (p) {
			qf32_mul((float*)g, (float const*)p, (float const*)g);
		}
	}
}

static void Position3_Cascade(ecs_iter_t *it)
{
	Position3 *l = ecs_field(it, Position3, 1); // self, in
	Position3World *g = ecs_field(it, Position3World, 2); // self, out
	Position3World *p = ecs_field(it, Position3World, 3); // parent, in
	for (int i = 0; i < it->count; ++i, ++l, ++g) {
		g[0].x += l[0].x;
		g[0].y += l[0].y;
		g[0].z += l[0].z;
		if (p) {
			g->x += p->x;
			g->y += p->y;
			g->z += p->z;
		}
	}
}

static void RotateQuaternion1(ecs_iter_t *it)
{
	Rotate3 *rotate = ecs_field(it, Rotate3, 1);              // self
	Orientation *orientation = ecs_field(it, Orientation, 2); // self
	for (int i = 0; i < it->count; ++i, ++rotate, ++orientation) {
		float *q = (float *)orientation;
		// assert(fabsf(V4_DOT(q, q) - 1.0f) < 0.1f);         // Check quaternion validity
		float dq_pitch[4];                                 // Quaternion delta pitch rotation
		float dq_yaw[4];                                   // Quaternion delta yaw rotation
		float dq_roll[4];                                  // Quaternion delta roll rotation
		qf32_normalize(q, q, 0.000001f);                   // Normalize quaternion against floating point error
		qf32_xyza(dq_pitch, 1.0f, 0.0f, 0.0f, rotate->dx); // Make delta pitch quaternion
		qf32_xyza(dq_yaw, 0.0f, 1.0f, 0.0f, rotate->dy);   // Make delta yaw quaternion
		qf32_xyza(dq_roll, 0.0f, 0.0f, 1.0f, rotate->dz);  // Make delta roll quaternion
		qf32_mul(q, q, dq_roll);                           // Apply roll delta rotation
		qf32_mul(q, q, dq_yaw);                            // Apply yaw delta rotation
		qf32_mul(q, q, dq_pitch);                          // Apply pitch delta rotation
	}
}

static void RotateQuaternion2(ecs_iter_t *it)
{
	Rotate3 *rotate = ecs_field(it, Rotate3, 1);              // self
	Orientation *orientation = ecs_field(it, Orientation, 2); // self
	for (int i = 0; i < it->count; ++i, ++rotate, ++orientation) {
		float *q = (float *)orientation;
		// assert(fabsf(V4_DOT(q, q) - 1.0f) < 0.1f);         // Check quaternion validity
		float dq_pitch[4];                                 // Quaternion delta pitch rotation
		float dq_yaw[4];                                   // Quaternion delta yaw rotation
		float dq_roll[4];                                  // Quaternion delta roll rotation
		qf32_normalize(q, q, 0.000001f);                   // Normalize quaternion against floating point error
		qf32_xyza(dq_pitch, 1.0f, 0.0f, 0.0f, rotate->dx); // Make delta pitch quaternion
		qf32_xyza(dq_yaw, 0.0f, 1.0f, 0.0f, rotate->dy);   // Make delta yaw quaternion
		qf32_xyza(dq_roll, 0.0f, 0.0f, 1.0f, rotate->dz);  // Make delta roll quaternion
		qf32_mul(q, dq_roll, q);                           // Apply roll delta rotation
		qf32_mul(q, dq_yaw, q);                            // Apply yaw delta rotation
		qf32_mul(q, dq_pitch, q);                          // Apply pitch delta rotation
	}
}

static void TransformationPosition(ecs_iter_t *it)
{
	Transformation *t = ecs_field(it, Transformation, 1);                     // self, out
	Position3World const *pos = ecs_field(it, Position3World, 2);             // self, in
	OrientationWorld const *orientation = ecs_field(it, OrientationWorld, 3); // self, in
	for (int i = 0; i < it->count; ++i, ++t, ++pos, ++orientation) {
		t->matrix = (m4f32)M4_IDENTITY;
		qf32_unit_to_m4((float *)orientation, &t->matrix);
		m4f32_translation3(&t->matrix, (float const *)pos);
	}
}

static void Move(ecs_iter_t *it)
{
	Position3 *p = ecs_field(it, Position3, 1);     // self
	Velocity3 *v = ecs_field(it, Velocity3, 2);     // self
	Orientation *o = ecs_field(it, Orientation, 3); // self

	for (int i = 0; i < it->count; ++i, ++p, ++o) {
		// Convert unit quaternion to rotation matrix (r)
		m4f32 r = M4_IDENTITY;
		qf32_unit_to_m4((float *)o, &r);
		// Translate postion (pos) relative to direction of camera rotation
		float dir[3];
		dir[0] = V3_DOT(r.c0, (float *)v);
		dir[1] = V3_DOT(r.c1, (float *)v);
		dir[2] = V3_DOT(r.c2, (float *)v);
		// v3f32_print((float*)dir);
		v3f32_add((float *)p, (float *)p, dir);
	}
}

static void SinewaveSystem(ecs_iter_t *it)
{
	Sinewave const *w = ecs_field(it, Sinewave, 1);
	Position3World *p = ecs_field(it, Position3World, 2);
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
	EulerAngles *e = ecs_field(it, EulerAngles, 1); // self, in
	Orientation *o = ecs_field(it, Orientation, 2); // self, out
	for (int i = 0; i < it->count; ++i, ++e, ++o) {
		qf32_from_euler((float*)o, e->pitch, e->yaw, e->roll);
	}
}


ECS_CTOR(Transformation, ptr, {
	ptr->matrix = (m4f32)M4_IDENTITY;
})


ECS_CTOR(RotMat3, ptr, {
	ptr->x1 = 1.0f;
	ptr->y1 = 0.0f;
	ptr->z1 = 0.0f;
	ptr->x2 = 0.0f;
	ptr->y2 = 1.0f;
	ptr->z2 = 0.0f;
	ptr->x3 = 0.0f;
	ptr->y3 = 0.0f;
	ptr->z3 = 1.0f;
})

ECS_CTOR(Orientation, ptr, {
	// QF32_IDENTITY;
	// printf("Orientation::ECS_CTOR\n");
	ptr->x = 0.0f;
	ptr->y = 0.0f;
	ptr->z = 0.0f;
	ptr->w = 1.0f;
})

ECS_CTOR(OrientationWorld, ptr, {
	// QF32_IDENTITY;
	// printf("Orientation::ECS_CTOR\n");
	ptr->x = 0.0f;
	ptr->y = 0.0f;
	ptr->z = 0.0f;
	ptr->w = 1.0f;
})


void SpatialsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Spatials);
	ecs_set_name_prefix(world, "Spatials");

	ECS_COMPONENT_DEFINE(world, Position2);
	ECS_COMPONENT_DEFINE(world, Position3);
	ECS_COMPONENT_DEFINE(world, Position3World);
	ECS_COMPONENT_DEFINE(world, Velocity2);
	ECS_COMPONENT_DEFINE(world, Velocity3);
	ECS_COMPONENT_DEFINE(world, Orientation);
	ECS_COMPONENT_DEFINE(world, OrientationWorld);
	ECS_COMPONENT_DEFINE(world, EulerAngles);
	ECS_COMPONENT_DEFINE(world, Rotate3);
	ECS_COMPONENT_DEFINE(world, Transformation);
	ECS_COMPONENT_DEFINE(world, RotMat3);
	ECS_COMPONENT_DEFINE(world, Sinewave);

	ECS_TAG_DEFINE(world, EgRotateOrder1);
	ECS_TAG_DEFINE(world, EgRotateOrder2);
	ECS_TAG_DEFINE(world, EgPositionWorldNoReset);

	ecs_set_hooks(world, Orientation, {.ctor = ecs_ctor(Orientation)});
	ecs_set_hooks(world, OrientationWorld, {.ctor = ecs_ctor(OrientationWorld)});
	ecs_set_hooks(world, Transformation, {.ctor = ecs_ctor(Transformation)});
	ecs_set_hooks(world, RotMat3, {.ctor = ecs_ctor(RotMat3)});

	ecs_struct(world,
	{.entity = ecs_id(Position2),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Position3),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Position3World),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	

	ecs_struct(world,
	{.entity = ecs_id(Velocity2),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Velocity3),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Orientation),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	{.name = "w", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(OrientationWorld),
	.members = {
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	{.name = "z", .type = ecs_id(ecs_f32_t)},
	{.name = "w", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EulerAngles),
	.members = {
	{.name = "pitch", .type = ecs_id(ecs_f32_t)},
	{.name = "yaw", .type = ecs_id(ecs_f32_t)},
	{.name = "roll", .type = ecs_id(ecs_f32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(Rotate3),
	.members = {
	{.name = "dx", .type = ecs_id(ecs_f32_t)},
	{.name = "dy", .type = ecs_id(ecs_f32_t)},
	{.name = "dz", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(Transformation),
	.members = {
	{.name = "matrix", .type = ecs_id(ecs_f32_t), .count = 16},
	}});

	ecs_struct(world,
	{.entity = ecs_id(RotMat3),
	.members = {
	{.name = "x1", .type = ecs_id(ecs_f32_t)},
	{.name = "y1", .type = ecs_id(ecs_f32_t)},
	{.name = "z1", .type = ecs_id(ecs_f32_t)},
	{.name = "x2", .type = ecs_id(ecs_f32_t)},
	{.name = "y2", .type = ecs_id(ecs_f32_t)},
	{.name = "z2", .type = ecs_id(ecs_f32_t)},
	{.name = "x3", .type = ecs_id(ecs_f32_t)},
	{.name = "y3", .type = ecs_id(ecs_f32_t)},
	{.name = "z3", .type = ecs_id(ecs_f32_t)}
	}});

	ecs_struct(world,
	{.entity = ecs_id(Sinewave),
	.members = {
	{.name = "frequency", .type = ecs_id(ecs_f32_t)},
	{.name = "amplitude", .type = ecs_id(ecs_f32_t)},
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Position3World_Reset", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Position3World_Reset,
	.query.filter.terms =
	{
	{.id = ecs_id(Position3World), .inout = EcsOut},
	{.id = EgPositionWorldNoReset, .oper = EcsNot},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "EulerToQ", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = EulerToQ,
	.query.filter.terms =
	{
	{.id = ecs_id(EulerAngles), .inout = EcsIn},
	{.id = ecs_id(Orientation), .inout = EcsOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Orientation_To_RotMat3", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Orientation_To_RotMat3,
	.query.filter.terms =
	{
	{.id = ecs_id(Orientation), .inout = EcsIn},
	{.id = ecs_id(RotMat3), .inout = EcsOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "RotateQuaternion1", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = RotateQuaternion1,
	.query.filter.terms = {
	{.id = ecs_id(Rotate3)},
	{.id = ecs_id(Orientation)},
	{.id = EgRotateOrder1},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "RotateQuaternion2", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = RotateQuaternion2,
	.query.filter.terms = {
	{.id = ecs_id(Rotate3)},
	{.id = ecs_id(Orientation)},
	{.id = EgRotateOrder2},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Move", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Move,
	.query.filter.terms = {
	{.id = ecs_id(Position3)},
	{.id = ecs_id(Velocity3)},
	{.id = ecs_id(Orientation)}
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "SinewaveSystem", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = SinewaveSystem,
	.query.filter.terms = {
	{.id = ecs_id(Sinewave), .inout = EcsIn},
	{.id = ecs_id(Position3World), .inout = EcsOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Orientation_Cascade", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Orientation_Cascade,
	.query.filter.terms =
	{
	{.id = ecs_id(Orientation), .inout = EcsIn},
	{.id = ecs_id(OrientationWorld), .inout = EcsOut},
	{.id = ecs_id(OrientationWorld), .src.flags = EcsParent | EcsCascade, .inout = EcsIn, .oper = EcsOptional},
	}});
 


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Position3_Cascade", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Position3_Cascade,
	.query.filter.terms =
	{
	{.id = ecs_id(Position3), .inout = EcsIn},
	{.id = ecs_id(Position3World), .inout = EcsOut},
	{.id = ecs_id(Position3World), .src.flags = EcsParent | EcsCascade, .inout = EcsIn, .oper = EcsOptional},
	}});



	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "TransformationPosition", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = TransformationPosition,
	.query.filter.terms = {
	{.id = ecs_id(Transformation), .src.flags = EcsSelf, .inout = EcsOut},
	{.id = ecs_id(Position3World), .src.flags = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(OrientationWorld), .src.flags = EcsSelf, .inout = EcsIn},
	}});



}