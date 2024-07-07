#include "egmath.h"

void qf32_xyza(float q[4], float x, float y, float z, float a)
{
	float const c = cosf(a * 0.5f);
	float const s = sinf(a * 0.5f);
	assert((c != 0) || (s != 0));
	q[0] = s * x;
	q[1] = s * y;
	q[2] = s * z;
	q[3] = c;
}

void qf32_normalize(float r[4], float const q[4], float epsilon)
{
	float l2 = V4_DOT(q, q);
	float l = sqrtf(l2 + epsilon);
	r[0] = q[0] / l;
	r[1] = q[1] / l;
	r[2] = q[2] / l;
	r[3] = q[3] / l;
}

void v3f32_normalize(float r[3], float const q[3], float epsilon)
{
	float l2 = V3_DOT(q, q);
	float l = sqrtf(l2 + epsilon);
	r[0] = q[0] / l;
	r[1] = q[1] / l;
	r[2] = q[2] / l;
}

void qf32_mul(float r[4], float const p[4], float const q[4])
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

void qf32_unit_to_m4(float const q[4], m4f32 *r)
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


/*
https://forums.inovaestudios.com/t/math-combining-a-translation-rotation-and-scale-matrix-edit-question-solved-by-math-magician-flavien/5194/4
https://old.reddit.com/r/Unity3D/comments/flwreg/how_do_i_make_a_trs_matrix_manually/
http://www.illusioncatalyst.com/notes_files/mathematics/line_nu_cylinder_intersection.php
*/
void m4f32_trs(float const t[3], float const q[4], float s[3], m4f32 *r)
{
	float x = q[0];
	float y = q[1];
	float z = q[2];
	float w = q[3];

	float sx = s[0];
	float sy = s[1];
	float sz = s[2];

	float xx = x * x;
	float yy = y * y;
	float zz = z * z;
	//float ww = w * w;

	float xy = x * y;
	float xz = x * z;
	float zw = z * w;
	float yw = y * w;
	float yz = y * z;
	float xw = x * w;

	r->c0[0] = (1.0f - 2.0f * (yy + zz)) * sx;
	r->c0[1] = (xy + zw) * sx * 2.0f;
	r->c0[2] = (xz - yw) * sx * 2.0f;
	r->c0[3] = 0.0f;

	r->c1[0] = (xy - zw) * sy * 2.0f;
	r->c1[1] = (1.0f - 2.0f * (xx + zz)) * sy;
	r->c1[2] = (yz + xw) * sy * 2.0f;
	r->c1[3] = 0.0f;

	r->c2[0] = (xz + yw) * sz * 2.0f;
	r->c2[1] = (yz - xw) * sz * 2.0f;
	r->c2[2] = (1.0f - 2.0f * (xx + yy)) * sz;
	r->c2[3] = 0.0f;

	r->c3[0] = t[0];
	r->c3[1] = t[1];
	r->c3[2] = t[2];
	r->c3[3] = 1.0f;
}

/*
http://www.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche0053.html
http://graphics.cs.cmu.edu/nsp/course/15-462/Spring04/slides/04-transform.pdf
https://math.stackexchange.com/questions/1234948/inverse-of-a-rigid-transformation
https://www.cuemath.com/algebra/inverse-of-diagonal-matrix/
*/
void m4f32_trs_inverse(float const t[3], float const q[4], float s[3], m4f32 *r)
{
	float x = q[0];
	float y = q[1];
	float z = q[2];
	float w = q[3];

	float sx = 1.0 / s[0];
	float sy = 1.0 / s[1];
	float sz = 1.0 / s[2];

	float xx = x * x;
	float yy = y * y;
	float zz = z * z;
	//float ww = w * w;

	float xy = x * y;
	float xz = x * z;
	float zw = z * w;
	float yw = y * w;
	float yz = y * z;
	float xw = x * w;

	// Store as transposed rotation matrix
	r->c0[0] = (1.0f - 2.0f * (yy + zz)) * sx;
	r->c1[0] = (xy + zw) * sx * 2.0f;
	r->c2[0] = (xz - yw) * sx * 2.0f;
	
	r->c0[1] = (xy - zw) * sy * 2.0f;
	r->c1[1] = (1.0f - 2.0f * (xx + zz)) * sy;
	r->c2[1] = (yz + xw) * sy * 2.0f;

	r->c0[2] = (xz + yw) * sz * 2.0f;
	r->c1[2] = (yz - xw) * sz * 2.0f;
	r->c2[2] = (1.0f - 2.0f * (xx + yy)) * sz;

	// Translation column:
	r->c3[0] = -V3_DOTE(t, M3_R0(*r));
	r->c3[1] = -V3_DOTE(t, M3_R1(*r));
	r->c3[2] = -V3_DOTE(t, M3_R2(*r));

	r->c0[3] = 0.0f;
	r->c1[3] = 0.0f;
	r->c2[3] = 0.0f;
	r->c3[3] = 1.0f;

}













/*

   public static Matrix4x4Custom TRS(Vector3d pos, Quaterniond q, Vector3d s)
   {
       Matrix4x4Custom result = new Matrix4x4Custom();
       // Rotation and Scale
       // Quaternion multiplication can be used to represent rotation.
       // If a quaternion is represented by qw + i qx + j qy + k qz , then the equivalent matrix for rotation is (including scale):
       // Remarks: https://forums.inovaestudios.com/t/math-combining-a-translation-rotation-and-scale-matrix-question-to-you-math-magicians/5194/2
       double sqw = q.w * q.w;
       double sqx = q.x * q.x;
       double sqy = q.y * q.y;
       double sqz = q.z * q.z;
       result.m00 = (float)(1 - 2 * sqy - 2 * sqz) * (float)s.x;
       result.m01 = (float)(2 * q.x * q.y - 2 * q.z * q.w);
       result.m02 = (float)(2 * q.x * q.z + 2 * q.y * q.w);
       result.m10 = (float)(2 * q.x * q.y + 2 * q.z * q.w);
       result.m11 = (float)(1 - 2 * sqx - 2 * sqz) * (float)s.y;
       result.m12 = (float)(2 * q.y * q.z - 2 * q.x * q.w);
       result.m20 = (float)(2 * q.x * q.z - 2 * q.y * q.w);
       result.m21 = (float)(2 * q.y * q.z + 2 * q.x * q.w);
       result.m22 = (float)(1 - 2 * sqx - 2 * sqy) * (float)s.z;
       // Translation
       result.m03 = (float)pos.x;
       result.m13 = (float)pos.y;
       result.m23 = (float)pos.z;
       result.m33 = 1.0f;
       // Return result
       return result;
   }
*/



void qf32_unit_to_m3(float const q[4], m3f32 *r)
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

void qf32_from_euler(float q[4], float pitch, float yaw, float roll)
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

void v3f32_cross(int const a[3], int const b[], int p[])
{
	p[0] = a[1] * b[2] - a[2] * b[1];
	p[1] = a[2] * b[0] - a[0] * b[2];
	p[2] = a[0] * b[1] - a[1] * b[0];
}

void qf32_rotate_vector(float const q[4], float const v[3], float output[3])
{
	assert(output != NULL);
	float result[3];
	float ww = q[3] * q[3];
	float xx = q[0] * q[0];
	float yy = q[1] * q[1];
	float zz = q[2] * q[2];
	float wx = q[3] * q[0];
	float wy = q[3] * q[1];
	float wz = q[3] * q[2];
	float xy = q[0] * q[1];
	float xz = q[0] * q[2];
	float yz = q[1] * q[2];

	// Formula from http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/index.htm
	// p2.x = w*w*p1.x + 2*y*w*p1.z - 2*z*w*p1.y + x*x*p1.x + 2*y*x*p1.y + 2*z*x*p1.z - z*z*p1.x - y*y*p1.x;
	// p2.y = 2*x*y*p1.x + y*y*p1.y + 2*z*y*p1.z + 2*w*z*p1.x - z*z*p1.y + w*w*p1.y - 2*x*w*p1.z - x*x*p1.y;
	// p2.z = 2*x*z*p1.x + 2*y*z*p1.y + z*z*p1.z - 2*w*y*p1.x - y*y*p1.z + 2*w*x*p1.y - x*x*p1.z + w*w*p1.z;

	result[0] = ww * v[0] + 2 * wy * v[2] - 2 * wz * v[1] +
	            xx * v[0] + 2 * xy * v[1] + 2 * xz * v[2] -
	            zz * v[0] - yy * v[0];
	result[1] = 2 * xy * v[0] + yy * v[1] + 2 * yz * v[2] +
	            2 * wz * v[0] - zz * v[1] + ww * v[1] -
	            2 * wx * v[2] - xx * v[1];
	result[2] = 2 * xz * v[0] + 2 * yz * v[1] + zz * v[2] -
	            2 * wy * v[0] - yy * v[2] + 2 * wx * v[1] -
	            xx * v[2] + ww * v[2];
	// Copy result to output
	output[0] = result[0];
	output[1] = result[1];
	output[2] = result[2];
}

void v3f32_mul(float r[3], float const a[3], float b)
{
	r[0] = a[0] * b;
	r[1] = a[1] * b;
	r[2] = a[2] * b;
}

void v3f32_add(float r[3], float const a[3], float const b[3])
{
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
}

void m4f32_perspective1(m4f32 *m, float fov, float aspect, float n, float f)
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

void m4f32_translation3(m4f32 *m, float const t[3])
{
	// Translation vector in 4th column
	m->c3[0] = t[0];
	m->c3[1] = t[1];
	m->c3[2] = t[2];
}

void m4f32_transpose(m4f32 *x)
{
	SWAP(float, x->c0[1], x->c1[0]);
	SWAP(float, x->c0[2], x->c2[0]);
	SWAP(float, x->c0[3], x->c3[0]);
	SWAP(float, x->c1[2], x->c2[1]);
	SWAP(float, x->c1[3], x->c3[1]);
	SWAP(float, x->c2[3], x->c3[2]);
}

void m4f32_mul_transpose(m4f32 *y, m4f32 const *at, m4f32 *const b)
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

void m4f32_mul(m4f32 *y, m4f32 const *a, m4f32 const *b)
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

void m3f32_mul(m3f32 *y, m3f32 const *a, m3f32 const *b)
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

void m4f32_mulv(m4f32 const *a, float const x[4], float y[4])
{
	y[0] = V4_DOTE(x, M4_R0(*a));
	y[1] = V4_DOTE(x, M4_R1(*a));
	y[2] = V4_DOTE(x, M4_R2(*a));
	y[3] = V4_DOTE(x, M4_R3(*a));
}

void m4f32_print(m4f32 const *x)
{
	printf("%+f %+f %+f %+f\n", M4_R0(*x));
	printf("%+f %+f %+f %+f\n", M4_R1(*x));
	printf("%+f %+f %+f %+f\n", M4_R2(*x));
	printf("%+f %+f %+f %+f\n", M4_R3(*x));
	printf("\n");
}

void v3f32_print(float const x[3])
{
	printf("%f %f %f\n", x[0], x[1], x[2]);
}

void v4f32_print(float const x[4])
{
	printf("%f %f %f %f\n", x[0], x[1], x[2], x[3]);
}

// https://github.com/travisvroman/kohi/blob/main/engine/src/math/kmath.h#L1203
void m4f32_inverse(float const m[16], float o[16])
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

/**
 * @brief Calculates the shortest distance between L1 and L2
 * https://math.stackexchange.com/questions/3081301/shortest-distance-between-two-vectors
 *
 * @param p1 Point on L1
 * @param v1 Direction of L1
 * @param p2 Point on L2
 * @param v2 Direction of L2
 * @return Returns the shortest distance between L1 and L2
 */
float v3f32_l1l2_distance(float const p1[3], float const v1[3], float const p2[3], float const v2[3])
{
	float d[3];
	d[0] = p1[0] - p2[0];
	d[1] = p1[1] - p2[1];
	d[2] = p1[2] - p2[2];
	float n[3];
	n[0] = V3_CROSS0(v1, v2);
	n[1] = V3_CROSS1(v1, v2);
	n[2] = V3_CROSS2(v1, v2);
	v3f32_normalize(n, n, 0.0001);
	float l = fabs(V3_DOT(d, n));
	return l;
}

void v3f32_proj(float const bnorm[3], float const a[3], float r[3])
{
	float k = V3_DOT(a, bnorm);
	r[0] = bnorm[0] * k;
	r[1] = bnorm[1] * k;
	r[2] = bnorm[2] * k;
}

void v3f32_oproj(float const bnorm[3], float const a[3], float r[3])
{
	float k = V3_DOT(a, bnorm);
	r[0] = a[0] - bnorm[0] * k;
	r[1] = a[1] - bnorm[1] * k;
	r[2] = a[2] - bnorm[2] * k;
}

float v3f32_distance(float const a[3], float const b[3])
{
	float d[3];
	d[0] = a[0] - b[0];
	d[1] = a[1] - b[1];
	d[2] = a[2] - b[2];
	return sqrtf(V3_DOT(d, d));
}

float v3f32_distance2(float const a[3], float const b[3])
{
	float d[3];
	d[0] = a[0] - b[0];
	d[1] = a[1] - b[1];
	d[2] = a[2] - b[2];
	return V3_DOT(d, d);
}

// http://www.illusioncatalyst.com/notes_files/mathematics/line_plane_intersection.php
float v3f32_plane_point_line_distance(float const v[3], float const l0[3], float const c[3], float const n[3])
{
	float w[3];
	w[0] = l0[0] - c[0];
	w[1] = l0[1] - c[1];
	w[2] = l0[2] - c[2];
	float vn = V3_DOT(v, n);
	if (vn == 0.0) {
		// The line is parallel to the plane.
		return 0;
	}

	float t = -V3_DOT(w, n) / V3_DOT(v, n);
	// There is one intersection at:
	float i[3];
	i[0] = l0[0] + t * v[0];
	i[1] = l0[1] + t * v[1];
	i[2] = l0[2] + t * v[2];
	return v3f32_distance(c, i);
}

float v3f32_intersect_line_circle(float const v[3], float const l0[3], float const c[3], float const n[3], float r)
{
	float w[3];
	w[0] = l0[0] - c[0];
	w[1] = l0[1] - c[1];
	w[2] = l0[2] - c[2];
	float vn = V3_DOT(v, n);
	if (vn == 0.0) {
		// The line is parallel to the plane.
		return 0;
	}
	float t = -V3_DOT(w, n) / V3_DOT(v, n);
	// There is one intersection at:
	float i[3];
	i[0] = l0[0] + t * v[0];
	i[1] = l0[1] + t * v[1];
	i[2] = l0[2] + t * v[2];
	return v3f32_distance2(c, i) < (r * r);
}

// // https://math.stackexchange.com/questions/2613781/line-cylinder-intersection
int v3f32_intersect_cylinder(float const v[3], float const l0[3], float const c[3], float const n[3], float r, float height)
{
	float cap[3];
	cap[0] = c[0] + n[0] * height * 0.5;
	cap[1] = c[1] + n[1] * height * 0.5;
	cap[2] = c[2] + n[2] * height * 0.5;
	float d1 = v3f32_plane_point_line_distance(v, l0, cap, n);
	cap[0] = c[0] + n[0] * height * -0.5;
	cap[1] = c[1] + n[1] * height * -0.5;
	cap[2] = c[2] + n[2] * height * -0.5;
	float d2 = v3f32_plane_point_line_distance(v, l0, cap, n);
	float d3 = v3f32_l1l2_distance(l0, v, c, n);
	printf("%f %f %f\n", d1, d2, d3);
}




void test_m4f32_trs()
{
	float t[3] = {1, 2, 3};
	float q[4];
	float s[3] = {4, 5, 6};
	qf32_from_euler(q, 123, 22, 34564567);
	m4f32 m1;
	m4f32 m2;
	m4f32 m3;
	m4f32_trs(t, q, s, &m1);
	m4f32_trs_inverse(t, q, s, &m2);
	m4f32_inverse((float*)&m1, (float*)&m3);
	m4f32_print(&m1);
	m4f32_print(&m2);
	m4f32_print(&m3);
}
