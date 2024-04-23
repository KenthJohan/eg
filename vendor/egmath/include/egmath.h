
#pragma once

#include <math.h>
#include <assert.h>
#include <stdio.h>

#define MAX(a, b) (((a) > (b)) ? a : b)
#define MIN(a, b) (((a) < (b)) ? a : b)

#define V1_DOT(a, b) ((a)[0] * (b)[0])
#define V2_DOT(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1])
#define V3_DOT(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define V4_DOT(a, b) ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2] + (a)[3] * (b)[3])

#define V1_DOTE_(a, b0) ((a)[0] * b0)
#define V2_DOTE_(a, b0, b1) ((a)[0] * b0 + (a)[1] * b1)
#define V3_DOTE_(a, b0, b1, b2) ((a)[0] * b0 + (a)[1] * b1 + (a)[2] * b2)
#define V4_DOTE_(a, b0, b1, b2, b3) ((a)[0] * b0 + (a)[1] * b1 + (a)[2] * b2 + (a)[3] * b3)

#define V1_DOTE(...) V1_DOTE_(__VA_ARGS__)
#define V2_DOTE(...) V2_DOTE_(__VA_ARGS__)
#define V3_DOTE(...) V3_DOTE_(__VA_ARGS__)
#define V4_DOTE(...) V4_DOTE_(__VA_ARGS__)

#define V1_ARG(a) (a[0])
#define V2_ARG(a) (a[0]), (a[1])
#define V3_ARG(a) (a[0]), (a[1]), (a[2])
#define V4_ARG(a) (a[0]), (a[1]), (a[2]), (a[3])

#define M3_R0(a) ((a).c0[0]), ((a).c1[0]), ((a).c2[0])
#define M3_R1(a) ((a).c0[1]), ((a).c1[1]), ((a).c2[1])
#define M3_R2(a) ((a).c0[2]), ((a).c1[2]), ((a).c2[2])

#define M4_R0(a) ((a).c0[0]), ((a).c1[0]), ((a).c2[0]), ((a).c3[0])
#define M4_R1(a) ((a).c0[1]), ((a).c1[1]), ((a).c2[1]), ((a).c3[1])
#define M4_R2(a) ((a).c0[2]), ((a).c1[2]), ((a).c2[2]), ((a).c3[2])
#define M4_R3(a) ((a).c0[3]), ((a).c1[3]), ((a).c2[3]), ((a).c3[3])

#define SWAP(T, a, b) \
	do {              \
		T tmp = a;    \
		a = b;        \
		b = tmp;      \
	} while (0)

typedef struct
{
	float c0[3];
	float c1[3];
	float c2[3];
} m3f32;

typedef struct
{
	float c0[4];
	float c1[4];
	float c2[4];
	float c3[4];
} m4f32;

#define QF32_IDENTITY \
	{                 \
		0, 0, 0, 1    \
	}

#define M3_IDENTITY                                              \
	{                                                            \
		{1, 0, 0}, {0, 1, 0}, {0, 0, 1} \
	}

#define M4_IDENTITY                                              \
	{                                                            \
		{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, { 0, 0, 0, 1 } \
	}

static void qf32_xyza(float q[4], float x, float y, float z, float a)
{
	float const c = cosf(a * 0.5f);
	float const s = sinf(a * 0.5f);
	assert((c != 0) || (s != 0));
	q[0] = s * x;
	q[1] = s * y;
	q[2] = s * z;
	q[3] = c;
}

static void qf32_normalize(float r[4], float const q[4], float epsilon)
{
	float l2 = V4_DOT(q, q);
	float l = sqrtf(l2 + epsilon);
	r[0] = q[0] / l;
	r[1] = q[1] / l;
	r[2] = q[2] / l;
	r[3] = q[3] / l;
}

static void v3f32_normalize(float r[3], float const q[3], float epsilon)
{
	float l2 = V3_DOT(q, q);
	float l = sqrtf(l2 + epsilon);
	r[0] = q[0] / l;
	r[1] = q[1] / l;
	r[2] = q[2] / l;
}

static void qf32_mul(float r[4], float const p[4], float const q[4])
{
	float t[4] = {0};
	t[0] = (p[3] * q[0]) + (p[0] * q[3]) + (p[1] * q[2]) - (p[2] * q[1]);
	t[1] = (p[3] * q[1]) - (p[0] * q[2]) + (p[1] * q[3]) + (p[2] * q[0]);
	t[2] = (p[3] * q[2]) + (p[0] * q[1]) - (p[1] * q[0]) + (p[2] * q[3]);
	t[3] = (p[3] * q[3]) - (p[0] * q[0]) - (p[1] * q[1]) - (p[2] * q[2]);
	r[0] = t[0];
	r[1] = t[1];
	r[2] = t[2];
	r[3] = t[3];
}

static void qf32_unit_to_m4(float const q[4], m4f32 *r)
{
	float a = q[3];
	float b = q[0];
	float c = q[1];
	float d = q[2];
	float a2 = a * a;
	float b2 = b * b;
	float c2 = c * c;
	float d2 = d * d;
	// Column vector 0:
	r->c0[0] = a2 + b2 - c2 - d2;
	r->c0[1] = 2.0f * (b * c + a * d);
	r->c0[2] = 2.0f * (b * d - a * c);
	// Column vector 1:
	r->c1[0] = 2.0f * (b * c - a * d);
	r->c1[1] = a2 - b2 + c2 - d2;
	r->c1[2] = 2.0f * (c * d + a * b);
	// Column vector 2:
	r->c2[0] = 2.0f * (b * d + a * c);
	r->c2[1] = 2.0f * (c * d - a * b);
	r->c2[2] = a2 - b2 - c2 + d2;
}

static void qf32_unit_to_m3(float const q[4], m3f32 *r)
{
	float a = q[3];
	float b = q[0];
	float c = q[1];
	float d = q[2];
	float a2 = a * a;
	float b2 = b * b;
	float c2 = c * c;
	float d2 = d * d;
	// Column vector 0:
	r->c0[0] = a2 + b2 - c2 - d2;
	r->c0[1] = 2.0f * (b * c + a * d);
	r->c0[2] = 2.0f * (b * d - a * c);
	// Column vector 1:
	r->c1[0] = 2.0f * (b * c - a * d);
	r->c1[1] = a2 - b2 + c2 - d2;
	r->c1[2] = 2.0f * (c * d + a * b);
	// Column vector 2:
	r->c2[0] = 2.0f * (b * d + a * c);
	r->c2[1] = 2.0f * (c * d - a * b);
	r->c2[2] = a2 - b2 - c2 + d2;
}


static void qf32_from_euler(float q[4], float pitch, float yaw, float roll)
{
    float cr = cos(roll * 0.5f);
    float sr = sin(roll * 0.5f);
    float cp = cos(pitch * 0.5f);
    float sp = sin(pitch * 0.5f);
    float cy = cos(yaw * 0.5f);
    float sy = sin(yaw * 0.5f);
    q[0] = cr * cp * cy + sr * sp * sy;
    q[1] = sr * cp * cy - cr * sp * sy;
    q[2] = cr * sp * cy + sr * cp * sy;
    q[3] = cr * cp * sy - sr * sp * cy;
}


static void v3f32_mul(float r[3], float const a[3], float b)
{
	r[0] = a[0] * b;
	r[1] = a[1] * b;
	r[2] = a[2] * b;
}

static void v3f32_add(float r[3], float const a[3], float const b[3])
{
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
}

static void m4f32_perspective1(m4f32 *m, float fov, float aspect, float n, float f)
{
	float a = 1.0f / tan(fov / 2.0f);
	// Column vector 0:
	m->c0[0] = a / aspect;
	m->c0[1] = 0.0f;
	m->c0[2] = 0.0f;
	m->c0[3] = 0.0f;
	// Column vector 1:
	m->c1[0] = 0.0f;
	m->c1[1] = a;
	m->c1[2] = 0.0f;
	m->c1[3] = 0.0f;
	// Column vector 2:
	m->c2[0] = 0.0f;
	m->c2[1] = 0.0f;
	m->c2[2] = -((f + n) / (f - n));
	m->c2[3] = -1.0f;
	// Column vector 3:
	m->c3[0] = 0.0f;
	m->c3[1] = 0.0f;
	m->c3[2] = -((2.0f * f * n) / (f - n));
	m->c3[3] = 0.0f;
}

static void m4f32_translation3(m4f32 *m, float const t[3])
{
	// Translation vector in 4th column
	m->c3[0] = t[0];
	m->c3[1] = t[1];
	m->c3[2] = t[2];
}

static void m4f32_transpose(m4f32 *x)
{
	SWAP(float, x->c0[1], x->c1[0]);
	SWAP(float, x->c0[2], x->c2[0]);
	SWAP(float, x->c0[3], x->c3[0]);
	SWAP(float, x->c1[2], x->c2[1]);
	SWAP(float, x->c1[3], x->c3[1]);
	SWAP(float, x->c2[3], x->c3[2]);
}

static void m4f32_mul_transpose(m4f32 *y, m4f32 const *at, m4f32 *const b)
{
	m4f32 t;
	// Column vector 0:
	t.c0[0] = V4_DOT(at->c0, b->c0);
	t.c1[0] = V4_DOT(at->c0, b->c1);
	t.c2[0] = V4_DOT(at->c0, b->c2);
	t.c3[0] = V4_DOT(at->c0, b->c3);
	// Column vector 1:
	t.c0[1] = V4_DOT(at->c1, b->c0);
	t.c1[1] = V4_DOT(at->c1, b->c1);
	t.c2[1] = V4_DOT(at->c1, b->c2);
	t.c3[1] = V4_DOT(at->c1, b->c3);
	// Column vector 2:
	t.c0[2] = V4_DOT(at->c2, b->c0);
	t.c1[2] = V4_DOT(at->c2, b->c1);
	t.c2[2] = V4_DOT(at->c2, b->c2);
	t.c3[2] = V4_DOT(at->c2, b->c3);
	// Column vector 3:
	t.c0[3] = V4_DOT(at->c3, b->c0);
	t.c1[3] = V4_DOT(at->c3, b->c1);
	t.c2[3] = V4_DOT(at->c3, b->c2);
	t.c3[3] = V4_DOT(at->c3, b->c3);
	*y = t;
}

static void m4f32_mul(m4f32 *y, m4f32 const *a, m4f32 const * b)
{
	m4f32 t;
	// Column vector 0:
	t.c0[0] = V4_DOTE(b->c0, M4_R0(*a));
	t.c1[0] = V4_DOTE(b->c1, M4_R0(*a));
	t.c2[0] = V4_DOTE(b->c2, M4_R0(*a));
	t.c3[0] = V4_DOTE(b->c3, M4_R0(*a));
	// Column vector 1:
	t.c0[1] = V4_DOTE(b->c0, M4_R1(*a));
	t.c1[1] = V4_DOTE(b->c1, M4_R1(*a));
	t.c2[1] = V4_DOTE(b->c2, M4_R1(*a));
	t.c3[1] = V4_DOTE(b->c3, M4_R1(*a));
	// Column vector 2:
	t.c0[2] = V4_DOTE(b->c0, M4_R2(*a));
	t.c1[2] = V4_DOTE(b->c1, M4_R2(*a));
	t.c2[2] = V4_DOTE(b->c2, M4_R2(*a));
	t.c3[2] = V4_DOTE(b->c3, M4_R2(*a));
	// Column vector 3:
	t.c0[3] = V4_DOTE(b->c0, M4_R3(*a));
	t.c1[3] = V4_DOTE(b->c1, M4_R3(*a));
	t.c2[3] = V4_DOTE(b->c2, M4_R3(*a));
	t.c3[3] = V4_DOTE(b->c3, M4_R3(*a));
	*y = t;
}

static void m3f32_mul(m3f32 *y, m3f32 const *a, m3f32 const * b)
{
	m3f32 t;
	// Column vector 0:
	t.c0[0] = V3_DOTE(b->c0, M3_R0(*a));
	t.c1[0] = V3_DOTE(b->c1, M3_R0(*a));
	t.c2[0] = V3_DOTE(b->c2, M3_R0(*a));
	// Column vector 1:
	t.c0[1] = V3_DOTE(b->c0, M3_R1(*a));
	t.c1[1] = V3_DOTE(b->c1, M3_R1(*a));
	t.c2[1] = V3_DOTE(b->c2, M3_R1(*a));
	// Column vector 2:
	t.c0[2] = V3_DOTE(b->c0, M3_R2(*a));
	t.c1[2] = V3_DOTE(b->c1, M3_R2(*a));
	t.c2[2] = V3_DOTE(b->c2, M3_R2(*a));
	*y = t;
}

static void m4f32_mulv(m4f32 const *a, float const x[4], float y[4])
{
	y[0] = V4_DOTE(x, M4_R0(*a));
	y[1] = V4_DOTE(x, M4_R1(*a));
	y[2] = V4_DOTE(x, M4_R2(*a));
	y[3] = V4_DOTE(x, M4_R3(*a));
}

static void m4f32_print(m4f32 const *x)
{
	printf("%f %f %f %f\n", M4_R0(*x));
	printf("%f %f %f %f\n", M4_R1(*x));
	printf("%f %f %f %f\n", M4_R2(*x));
	printf("%f %f %f %f\n", M4_R3(*x));
	printf("\n");
}

static void v3f32_print(float const x[3])
{
	printf("%f %f %f\n", x[0], x[1], x[2]);
}

static void v4f32_print(float const x[4])
{
	printf("%f %f %f %f\n", x[0], x[1], x[2], x[3]);
}


//https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h#L1203
static void m4f32_inverse(float const m[16], float o[16])
{
	float t0 = m[10] * m[15];
	float t1 = m[14] * m[11];
	float t2 = m[6] * m[15];
	float t3 = m[14] * m[7];
	float t4 = m[6] * m[11];
	float t5 = m[10] * m[7];
	float t6 = m[2] * m[15];
	float t7 = m[14] * m[3];
	float t8 = m[2] * m[11];
	float t9 = m[10] * m[3];
	float t10 = m[2] * m[7];
	float t11 = m[6] * m[3];
	float t12 = m[8] * m[13];
	float t13 = m[12] * m[9];
	float t14 = m[4] * m[13];
	float t15 = m[12] * m[5];
	float t16 = m[4] * m[9];
	float t17 = m[8] * m[5];
	float t18 = m[0] * m[13];
	float t19 = m[12] * m[1];
	float t20 = m[0] * m[9];
	float t21 = m[8] * m[1];
	float t22 = m[0] * m[5];
	float t23 = m[4] * m[1];

	o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) -
	       (t1 * m[5] + t2 * m[9] + t5 * m[13]);
	o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) -
	       (t0 * m[1] + t7 * m[9] + t8 * m[13]);
	o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) -
	       (t3 * m[1] + t6 * m[5] + t11 * m[13]);
	o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) -
	       (t4 * m[1] + t9 * m[5] + t10 * m[9]);

	float d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

	o[0] = d * o[0];
	o[1] = d * o[1];
	o[2] = d * o[2];
	o[3] = d * o[3];
	o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) -
	            (t0 * m[4] + t3 * m[8] + t4 * m[12]));
	o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) -
	            (t1 * m[0] + t6 * m[8] + t9 * m[12]));
	o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) -
	            (t2 * m[0] + t7 * m[4] + t10 * m[12]));
	o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) -
	            (t5 * m[0] + t8 * m[4] + t11 * m[8]));
	o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) -
	            (t13 * m[7] + t14 * m[11] + t17 * m[15]));
	o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) -
	            (t12 * m[3] + t19 * m[11] + t20 * m[15]));
	o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) -
	             (t15 * m[3] + t18 * m[7] + t23 * m[15]));
	o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) -
	             (t16 * m[3] + t21 * m[7] + t22 * m[11]));
	o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) -
	             (t16 * m[14] + t12 * m[6] + t15 * m[10]));
	o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) -
	             (t18 * m[10] + t21 * m[14] + t13 * m[2]));
	o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) -
	             (t22 * m[14] + t14 * m[2] + t19 * m[6]));
	o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) -
	             (t20 * m[6] + t23 * m[10] + t17 * m[2]));
}