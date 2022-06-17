#include "EgCamera.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "eg_basics.h"
#include "eg_lin.h"


ECS_COMPONENT_DECLARE(EgCamera3DKeyBindings);
ECS_COMPONENT_DECLARE(EgCamera3D);
ECS_COMPONENT_DECLARE(EgCamera2D);
ECS_COMPONENT_DECLARE(EgScene);




static void System_Camera3D_Controller(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	EgCamera3D            *c = ecs_term(it, EgCamera3D, 1);
	EgCamera3DKeyBindings *k = ecs_term(it, EgCamera3DKeyBindings, 2);
	EgUserEvent           *u = ecs_term(it, EgUserEvent, 3);
	for (int i = 0; i < it->count; i ++)
	{
		c[i].look[0] = EG_U64BITSET_GET(u[i].keyboard, k[i].pitch[0]) - EG_U64BITSET_GET(u[i].keyboard, k[i].pitch[1]);
		c[i].look[1] = EG_U64BITSET_GET(u[i].keyboard, k[i].yaw[0])   - EG_U64BITSET_GET(u[i].keyboard, k[i].yaw[1]);
		c[i].look[2] = EG_U64BITSET_GET(u[i].keyboard, k[i].roll[0])  - EG_U64BITSET_GET(u[i].keyboard, k[i].roll[1]);
		c[i].move[0] = EG_U64BITSET_GET(u[i].keyboard, k[i].dx[0]) - EG_U64BITSET_GET(u[i].keyboard, k[i].dx[1]);
		c[i].move[1] = EG_U64BITSET_GET(u[i].keyboard, k[i].dy[0]) - EG_U64BITSET_GET(u[i].keyboard, k[i].dy[1]);
		c[i].move[2] = EG_U64BITSET_GET(u[i].keyboard, k[i].dz[0]) - EG_U64BITSET_GET(u[i].keyboard, k[i].dz[1]);
	}
}

static void System_Camera3D_Controller1(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	EgCamera3D       *comp_c = ecs_term(it, EgCamera3D, 1);
	EgVelocity3F32   *comp_v = ecs_term(it, EgVelocity3F32, 2);
	EgQuaternionF32  *comp_q = ecs_term(it, EgQuaternionF32, 3);
	for (int i = 0; i < it->count; i ++)
	{
		v3f32 * v = (v3f32*)(comp_v + i);
		qf32 * q = (qf32*)(comp_q + i);
		v3f32 * move = (v3f32 *)comp_c[i].move;
		v3f32 * look = (v3f32 *)comp_c[i].look;

		qf32 q_pitch; // Quaternion pitch rotation
		qf32 q_yaw;   // Quaternion yaw rotation
		qf32 q_roll;  // Quaternion roll rotation
		qf32_xyza (&q_pitch, 1.0f, 0.0f, 0.0f, look->x);
		qf32_xyza (&q_yaw,   0.0f, 1.0f, 0.0f, look->y);
		qf32_xyza (&q_roll,  0.0f, 0.0f, 1.0f, look->z);

		qf32_mul (q, &q_roll, q);  // Apply roll rotation
		qf32_mul (q, &q_yaw, q);   // Apply yaw rotation
		qf32_mul (q, &q_pitch, q); // Apply pitch rotation

		qf32_normalize (q, q); //Normalize quaternion against floating point error

		m4f32 mr;
		qf32_unit_m4 (&mr, q);
		v3f32_m4_mul (v, &mr, move);
	}
}


void EgCameraImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCamera);
	ECS_IMPORT(world, EgEvents);
	ECS_IMPORT(world, EgQuantities);

	ECS_COMPONENT_DEFINE(world, EgCamera3DKeyBindings);
	ECS_COMPONENT_DEFINE(world, EgCamera3D);
	ECS_COMPONENT_DEFINE(world, EgCamera2D);
	ECS_COMPONENT_DEFINE(world, EgScene);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgScene),
	.members = {
	{ .name = "a", .type = ecs_id(ecs_string_t) },
	{ .name = "enabled_depth", .type = ecs_id(ecs_bool_t) },
	{ .name = "enabled_facecull", .type = ecs_id(ecs_bool_t) },
	{ .name = "query", .type = ecs_id(ecs_uptr_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgCamera3DKeyBindings),
	.members = {
	{ .name = "dx", .type = ecs_id(ecs_i32_t), 2 },
	{ .name = "dy", .type = ecs_id(ecs_i32_t), 2 },
	{ .name = "dz", .type = ecs_id(ecs_i32_t), 2 },
	{ .name = "pitch", .type = ecs_id(ecs_i32_t), 2 },
	{ .name = "yaw", .type = ecs_id(ecs_i32_t), 2 },
	{ .name = "roll", .type = ecs_id(ecs_i32_t), 2 },
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgCamera3D),
	.members = {
	{ .name = "dp", .type = ecs_id(ecs_f32_t), 3 },
	{ .name = "dr", .type = ecs_id(ecs_f32_t), 3 },
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgCamera2D),
	.members = {
	{ .name = "forward", .type = ecs_id(ecs_i32_t) },
	{ .name = "backward", .type = ecs_id(ecs_i32_t) },
	{ .name = "left", .type = ecs_id(ecs_i32_t) },
	{ .name = "right", .type = ecs_id(ecs_i32_t) },
	}
	});

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgCamera3D, EgVelocity3F32, EgQuaternionF32",
	.entity.add = {EcsOnUpdate},
	.callback = System_Camera3D_Controller1});

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgCamera3D, EgCamera3DKeyBindings, EgUserEvent($)",
	.entity.add = {EcsOnUpdate},
	.callback = System_Camera3D_Controller});



}

