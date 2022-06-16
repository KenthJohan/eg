#include "EgCamera.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "eg_basics.h"
#include "eg_lin.h"


ECS_COMPONENT_DECLARE(EgCamera3D);
ECS_COMPONENT_DECLARE(EgCamera2D);
ECS_COMPONENT_DECLARE(EgScene);




/*

	gs_platform_t* platform = gs_subsystem(platform);
	gs_vec3 dp;
	const float mod = gs_platform_key_down(GS_KEYCODE_LEFT_SHIFT) ? 2.f : 1.f;
	dp.x = gs_platform_key_down(GS_KEYCODE_LEFT) - gs_platform_key_down(GS_KEYCODE_RIGHT);
	dp.y = gs_platform_key_down(GS_KEYCODE_UP) - gs_platform_key_down(GS_KEYCODE_DOWN);
	dp.z = gs_platform_key_down(GS_KEYCODE_Q) - gs_platform_key_down(GS_KEYCODE_E);
	//gs_camera_offset_orientation1(cam, dp.x, dp.y, dp.z);

	gs_quat x = gs_quat_angle_axis(gs_deg2rad(dp.x), gs_v3(0.f, 1.f, 0.f));
	gs_quat y = gs_quat_angle_axis(gs_deg2rad(dp.y), gs_v3(1.f, 0.f, 0.f));
	gs_quat z = gs_quat_angle_axis(gs_deg2rad(dp.z), gs_v3(0.f, 0.f, 1.f));
	q[i] = gs_quat_mul(q[i], y);
	q[i] = gs_quat_mul(q[i], x);
	q[i] = gs_quat_mul(q[i], z);

	gs_vec3 vel = {0};
	if (gs_platform_key_down(GS_KEYCODE_W))            vel = gs_quat_rotate(q[i], gs_v3( 0.0f,  0.0f, -1.0f));
	if (gs_platform_key_down(GS_KEYCODE_S))            vel = gs_quat_rotate(q[i], gs_v3( 0.0f,  0.0f,  1.0f));
	if (gs_platform_key_down(GS_KEYCODE_A))            vel = gs_quat_rotate(q[i], gs_v3(-1.0f,  0.0f,  0.0f));
	if (gs_platform_key_down(GS_KEYCODE_D))            vel = gs_quat_rotate(q[i], gs_v3( 1.0f,  0.0f,  0.0f));
	if (gs_platform_key_down(GS_KEYCODE_SPACE))        vel = gs_quat_rotate(q[i], gs_v3( 0.0f,  1.0f,  0.0f));
	if (gs_platform_key_down(GS_KEYCODE_LEFT_CONTROL)) vel = gs_quat_rotate(q[i], gs_v3( 0.0f, -1.0f,  0.0f));

	float cam_speed = 5.0f;
	p[i] = gs_vec3_add(p[i], gs_vec3_scale(gs_vec3_norm(vel), dt * cam_speed * mod));
*/


static void System_Camera3D_Controller(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	EgCamera3D *c = ecs_term(it, EgCamera3D, 1);
	EgUserEvent *u = ecs_term(it, EgUserEvent, 2);
	EgVelocity3F32 *comp_v = ecs_term(it, EgVelocity3F32, 3);
	EgQuaternionF32 *comp_q = ecs_term(it, EgQuaternionF32, 4);
	for (int i = 0; i < it->count; i ++)
	{
		float d[3];
		float * v = (float*)(comp_v + i);
		float * q = (float*)(comp_q + i);

		d[0] = EG_U64BITSET_GET(u[i].keyboard, c[i].pitch[0]) - EG_U64BITSET_GET(u[i].keyboard, c[i].pitch[1]);
		d[1] = EG_U64BITSET_GET(u[i].keyboard, c[i].yaw[0]) - EG_U64BITSET_GET(u[i].keyboard, c[i].yaw[1]);
		d[2] = EG_U64BITSET_GET(u[i].keyboard, c[i].roll[0]) - EG_U64BITSET_GET(u[i].keyboard, c[i].roll[1]);

		float q_pitch[4];
		float q_yaw[4];
		float q_roll[4];
		qf32_angle_axis(d[0], (float[3]){0.0f, 1.0f, 0.0f}, q_pitch);
		qf32_angle_axis(d[1], (float[3]){1.0f, 0.0f, 0.0f}, q_yaw);
		qf32_angle_axis(d[2], (float[3]){0.0f, 0.0f, 1.0f}, q_roll);

		qf32_mul(q, q_pitch, q);
		qf32_mul(q, q_yaw, q);
		qf32_mul(q, q_roll, q);
		v3f32_normalize(q);
		printf(V4F32_FORMAT"\n", V4F32_ARGS(q));

		d[0] = EG_U64BITSET_GET(u[i].keyboard, c[i].dx[0]) - EG_U64BITSET_GET(u[i].keyboard, c[i].dx[1]);
		d[1] = EG_U64BITSET_GET(u[i].keyboard, c[i].dy[0]) - EG_U64BITSET_GET(u[i].keyboard, c[i].dy[1]);
		d[2] = EG_U64BITSET_GET(u[i].keyboard, c[i].dz[0]) - EG_U64BITSET_GET(u[i].keyboard, c[i].dz[1]);
		v3f32_normalize(d);
		qf32_rotate(q, d, v);
	}
}


void EgCameraImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCamera);
	ECS_IMPORT(world, EgEvents);
	ECS_IMPORT(world, EgQuantities);

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
	.entity.entity = ecs_id(EgCamera3D),
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
	.entity.entity = ecs_id(EgCamera2D),
	.members = {
	{ .name = "forward", .type = ecs_id(ecs_i32_t) },
	{ .name = "backward", .type = ecs_id(ecs_i32_t) },
	{ .name = "left", .type = ecs_id(ecs_i32_t) },
	{ .name = "right", .type = ecs_id(ecs_i32_t) },
	}
	});

	ecs_system_init(world, &(ecs_system_desc_t) {
	.query.filter.expr = "EgCamera3D, EgUserEvent($), EgVelocity3F32, EgQuaternionF32",
	/*
	.query.filter.terms = {
	{ .id = ecs_id(EgCamera3D), .inout = EcsInOut },
	{ .id = ecs_id(EgUserEvent), .inout = EcsInOut},
	{ .id = ecs_id(EgVelocity3F32), .inout = EcsInOut},
	{ .id = ecs_id(EgQuaternionF32), .inout = EcsInOut}
	},
	*/
	.entity.add = {EcsOnUpdate},
	.callback = System_Camera3D_Controller
});



}

