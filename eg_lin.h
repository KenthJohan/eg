#pragma once
#include <math.h>


#define V3F32_FORMAT "(%f %f %f)"
#define V4F32_FORMAT "(%f %f %f %f)"
#define V3F32_ARGS(x) (x)[0], (x)[1], (x)[2]
#define V4F32_ARGS(x) (x)[0], (x)[1], (x)[2], (x)[3]

static void v3f32_cross(float v0[3], float v1[3], float r[3])
{
	r[0] = v0[1] * v1[2] - v0[2] * v1[1];
	r[1] = v0[2] * v1[0] - v0[0] * v1[2];
	r[2] = v0[0] * v1[1] - v0[1] * v1[0];
}


static float v3f32_dot(float a[3], float b[3])
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

static void v3f32_normalize(float a[3])
{
	float d2 = v3f32_dot(a, a);
	float l = sqrtf(d2);
	l = (l == 0.0f) ? 1.0f : l;
	a[0] /= l;
	a[1] /= l;
	a[2] /= l;
}


static void qf32_mul(float q0[4], float q1[4], float r[4])
{
	float t[4];
	t[0] = q0[3] * q1[0] + q1[3] * q0[0] + q0[1] * q1[2] - q1[1] * q0[2];
	t[1] = q0[3] * q1[1] + q1[3] * q0[1] + q0[2] * q1[0] - q1[2] * q0[0];
	t[2] = q0[3] * q1[2] + q1[3] * q0[2] + q0[0] * q1[1] - q1[0] * q0[1];
	t[3] = q0[3] * q1[3] - q0[0] * q1[0] - q0[1] * q1[1] - q0[2] * q1[2];
	r[0] = t[0];
	r[1] = t[1];
	r[2] = t[2];
	r[3] = t[3];
}

static void qf32_angle_axis(float rad, float axis[3], float r[4])
{
	v3f32_normalize(axis);
	rad *= 0.5;
	float s = sin(rad);
	r[0] = axis[0] * s;
	r[1] = axis[1] * s;
	r[2] = axis[2] * s;
	r[3] = cos(rad);
}


/*
	gs_vec3 qvec = gs_vec3_ctor(q.x, q.y, q.z);
	gs_vec3 uv = gs_vec3_cross(qvec, v);
	gs_vec3 uuv = gs_vec3_cross(qvec, uv);
	uv = gs_vec3_scale(uv, 2.f * q.w);
	uuv = gs_vec3_scale(uuv, 2.f);
	return (gs_vec3_add(v, gs_vec3_add(uv, uuv)));
*/
static void qf32_rotate(float q[4], float v[3], float r[3])
{
	float uv[3];
	float uuv[3];
	v3f32_cross(q, v, uv);
	v3f32_cross(q, uv, uuv);
	r[0] = (uv[0] * q[3]) + (uuv[0] * 2.0f);
	r[1] = (uv[1] * q[3]) + (uuv[1] * 2.0f);
	r[2] = (uv[2] * q[3]) + (uuv[2] * 2.0f);
}
