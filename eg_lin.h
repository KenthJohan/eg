#pragma once
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include "eg_types.h"

#ifndef ASSERT
#define ASSERT(x, ...)
#endif

#ifndef ASSERT_NEQP
#define ASSERT_NEQP(a, b)
#endif




// r := a < b
static void vvf32_lt (uint32_t n, float r [], float const a [], float const b [])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = a [i] < b [i];
	}
}


// r := a > b
static void vvf32_gt (uint32_t n, float r [], float const a [], float const b [])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = a [i] > b [i];
	}
}


// r := a + b
static void vvf32_add (uint32_t n, float r [], float const a [], float const b [])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = a [i] + b [i];
	}
}


// r := a - b
static void vvf32_sub (uint32_t n, float r [], float const a [], float const b [])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = a [i] - b [i];
	}
}


static void vf32_addv (uint32_t dim, float y[], uint32_t y_stride, float const a[], uint32_t a_stride, float const b[], uint32_t b_stride, uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		vvf32_add (dim, y, a, b);
		y += y_stride;
		a += a_stride;
		b += b_stride;
	}
}


static void vf32_subv (uint32_t dim, float y[], uint32_t y_stride, float const a[], uint32_t a_stride, float const b[], uint32_t b_stride, uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		vvf32_sub (dim, y, a, b);
		y += y_stride;
		a += a_stride;
		b += b_stride;
	}
}


static void vf32_set1_strided (float v[], float x, uint32_t n, unsigned inc)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		v[0] = x;
		v += inc;
	}
}


static void vf32_set2 (float v[], float v0, float v1)
{
	v[0] = v0;
	v[1] = v1;
}


static void vf32_set3 (float v[], float v0, float v1, float v2)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
}


static void vf32_set4 (float v[], float v0, float v1, float v2, float v3)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	v[3] = v3;
}



// r := a . b
static float vf32_dot (uint32_t n, float const a [], float const b [])
{
	float r = 0.0f;
	for (uint32_t i = 0; i < n; ++i)
	{
		r += a[i] * b[i];
	}
	return r;
}



static void vf32_cpy (uint32_t n, float des[], float const src[])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		des[i] = src[i];
	}
}







// r := r + a
static void vf32_acc (unsigned n, float r [], float const a [])
{
	vvf32_add (n, r, r, a);
}


// r := r - a
static void vf32_decc (unsigned n, float r [], float const a [])
{
	vvf32_sub (n, r, r, a);
}





// ret a > b
static int vf32_gt (uint32_t n, float const a [], float const b [])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		if (a[i] < b[i])
		{
			return 0;
		}
	}
	return 1;
}


// ret a < b
static int vf32_lt (unsigned n, float const a[], float const b[])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		if (a[i] < b[i]) {return 0;}
	}
	return 1;
}


// Set all element (x) of r to b
// r := {x | x = ?}
static void vf32_random (uint32_t n, float r [])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = (float)rand() / (float)RAND_MAX;
		//r [i] = 1.0f;
	}
}


// Set all element (x) of r to b
// r := {x | x = b}
static void vf32_set1 (uint32_t n, float r [], float const b)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r[i] = b;
	}
}


// r := a * b
static void vvf32_hadamard (unsigned n, float r[], float const a[], float const b[])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r[i] = a[i] * b[i];
	}
}


// r := r + a * b
static void vvf32_macc (uint32_t n, float r [], float const a [], float const b [])
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r[i] += a[i] * b[i];
	}
}


static float vf32_sum (uint32_t n, float const v [])
{
	float sum = 0;
	for (uint32_t i = 0; i < n; ++i)
	{
		sum += v[i];
	}
	return sum;
}










static void vf32_linespace (uint32_t n, float x[], float x1, float x2)
{
	float const d = (x2 - x1) / n;
	float a = x1;
	for (uint32_t i = 0; i < n; ++i)
	{
		x[i] = a;
		a += d;
	}
}


static void vf32_repeat (uint32_t n, float v[], float value, uint32_t stride)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		v[i*stride] = value;
	}
}


static void vf32_weight_ab (uint32_t n, float y[], float a[], float b[], float k)
{
	float A = k;
	float B = 1.0f - k;
	for (uint32_t i = 0; i < n; ++i)
	{
		y[i] = A * a[i] + B * b[i];
	}
}


static void vf32_setl (float r [], uint32_t n, ...)
{
	va_list ap;
	va_start (ap, n);
	for (uint32_t i = 0; i < n; ++i)
	{
		r[i] = va_arg (ap, double);
	}
	va_end (ap);
}


static float vf32_avg (uint32_t n, float v[])
{
	ASSERT (n > 0);
	float sum = 0.0f;
	for (uint32_t i = 0; i < n; ++i)
	{
		sum += v[i];
	}
	return sum / n;
}



static float vf32_max (uint32_t n, float v[])
{
	ASSERT (n >= 1);
	float max = v[0];
	for (uint32_t i = 0; i < n; ++i)
	{
		if (v[i] > max)
		{
			max = v[i];
		}
	}
	return max;
}


static float vf32_maxabs (uint32_t n, float v[])
{
	ASSERT (n >= 1);
	float max = v[0];
	for (uint32_t i = 0; i < n; ++i)
	{
		float x = fabs (v[i]);
		if (x > max)
		{
			max = x;
		}
	}
	return max;
}
























// r := a - b
static void vsf32_sub (unsigned n, float r [], float const a [], float b)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = a [i] - b;
	}
}

static void vsf32_macc (uint32_t n, float vy[], float const vx[], float sb)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		vy [i] += vx [i] * sb;
	}
}

// r := a + b
static void vsf32_add (uint32_t n, float r [], float const a [], float const b)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = a [i] + b;
	}
}

// r := a + b
static void vsf32_add_max (uint32_t n, float r [], float const a [], float const b, float max)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		if (r [i] < max)
		{
			r [i] = a [i] + b;
		}
	}
}

// r := a * b
static void vsf32_mul (uint32_t n, float r [], float const a [], float const b)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		r [i] = a [i] * b;
	}
}





















// ret |a|^2
static float vf32_norm2 (uint32_t n, float const a [])
{
	return vf32_dot (n, a, a);
}


// ret |a|
static float vf32_norm (uint32_t n, float const a [])
{
	return sqrtf (vf32_norm2 (n, a));
}


// r := a / |a|
static void vf32_normalize (uint32_t n, float r [], float const a [])
{
	float const l = vf32_norm (n, a);
	float const s = l > 0.0f ? 1.0f / l : 0.0f;
	vsf32_mul (n, r, a, s);
}




static void v3f32_set1 (v3f32 * r, float b)
{
	r->x = b;
	r->y = b;
	r->z = b;
}


static void v3f32_set_xyz (v3f32 * r, float x, float y, float z)
{
	r->x = x;
	r->y = y;
	r->z = z;
}


static void v3f32_cpy (v3f32 * r, v3f32 const * a)
{
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
}


static void v3f32_mul (v3f32 * r, v3f32 const * a, float b)
{
	r->x = a->x * b;
	r->y = a->y * b;
	r->z = a->z * b;
}

static void v3f32_mul_hadamard (v3f32 * r, v3f32 const * a, v3f32 const * b)
{
	r->x = a->x * b->x;
	r->y = a->y * b->y;
	r->z = a->z * b->z;
}


static void v3f32_sub (v3f32 * r, v3f32 const * a, v3f32 const * b)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
}

static void v3f32_subv (v3f32 r[], v3f32 const a[], v3f32 const b[], uint32_t incr, uint32_t inca, uint32_t incb, uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		v3f32_sub (r, a, b);
		r += incr;
		a += inca;
		b += incb;
	}
}

static void v3f32_add (v3f32 * r, v3f32 const * a, v3f32 const * b)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
}

static void v3f32_addv (v3f32 r[], v3f32 const a[], v3f32 const b[], uint32_t incr, uint32_t inca, uint32_t incb, uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		v3f32_add (r, a, b);
		r += incr;
		a += inca;
		b += incb;
	}
}


static void v3f32_add_mul (v3f32 * r, v3f32 const * a, v3f32 const * b, float alpha, float beta)
{
	r->x = a->x * alpha + b->x * beta;
	r->y = a->y * alpha + b->y * beta;
	r->z = a->z * alpha + b->z * beta;
}


static void v3f32_cross (v3f32 * r, v3f32 const * a, v3f32 const * b)
{
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->z * b->x - a->x * b->z;
	r->z = a->x * b->y - a->y * b->x;
}


static float v3f32_dot (v3f32 const * a, v3f32 const * b)
{
	float sum = 0;
	sum += a->x * b->x;
	sum += a->y * b->y;
	sum += a->z * b->z;
	return sum;
}


static float v3f32_dotv (float r[], v3f32 const a[], v3f32 const b[], int inca, int incb, int count)
{
	float sum = 0;
	for (int i = 0; i < count; ++i)
	{
		r[i] = (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
		a += inca;
		b += incb;
	}
	return sum;
}



static void v3f32_crossacc (v3f32 * r, v3f32 const * a, v3f32 const * b)
{
	r->x += a->y * b->z - a->z * b->y;
	r->y += a->z * b->x - a->x * b->z;
	r->z += a->x * b->y - a->y * b->x;
}


static void v3f32_crossacc_scalar (v3f32 * r, float s, v3f32 const * a, v3f32 const * b)
{
	r->x += s * (a->y * b->z - a->z * b->y);
	r->y += s * (a->z * b->x - a->x * b->z);
	r->z += s * (a->x * b->y - a->y * b->x);
}


static void v3f32_sum (v3f32 * y, v3f32 x[], uint32_t x_stride, uint32_t x_count)
{
	for (uint32_t i = 0; i < x_count; ++i)
	{
		y->x += x->x;
		y->y += x->y;
		y->z += x->z;
		x += x_stride;
	}
}


static float v3f32_norm2 (v3f32 const * a)
{
	return v3f32_dot (a, a);
}

static float v3f32_norm (v3f32 const * a)
{
	return sqrtf (v3f32_norm2 (a));
}

static void v3f32_normalize (v3f32 * a)
{
	float l = v3f32_norm (a);
	ASSERT (l != 0.0f);
	a->x /= l;
	a->y /= l;
	a->z /= l;
}




static int v3f32_ray_sphere_intersect (v3f32 * p, v3f32 * d, v3f32 * sc, float sr, float *t, v3f32 * q)
{
	//Vector m = p - s.c;
	v3f32 m;
	v3f32_sub (&m, p, sc);
	//float b = Dot(m, d);
	float b = v3f32_dot (&m, d);
	//float c = Dot(m, m) - s.r * s.r;
	float c = v3f32_dot (&m, &m) - (sr * sr);

	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
	if (c > 0.0f && b > 0.0f) {return 0;}
	float discr = (b * b) - c;

	// A negative discriminant corresponds to ray missing sphere
	if (discr < 0.0f) {return 0;}

	// Ray now found to intersect sphere, compute smallest t value of intersection
	(*t) = -b - sqrtf (discr);

	// If t is negative, ray started inside sphere so clamp t to zero
	if (*t < 0.0f) {*t = 0.0f;}
	(*t) = ((*t) > 0.0f) ? (*t) : 0.0f;

	//q = p + dt;
	v3f32_mul (q, d, *t);
	v3f32_add (q, q, p);

	return 1;
}


static void v3f32_m4_mul (v3f32 * y, m4f32 const * a, v3f32 * const b)
{
	y->x = (a->m11 * b->x) + (a->m21 * b->y) + (a->m31 * b->z);
	y->y = (a->m12 * b->x) + (a->m22 * b->y) + (a->m32 * b->z);
	y->z = (a->m13 * b->x) + (a->m23 * b->y) + (a->m33 * b->z);
}


static void v3f32_m3_mul (v3f32 * y, m3f32 const * a, v3f32 * const b)
{
	y->x = (a->m11 * b->x) + (a->m21 * b->y) + (a->m31 * b->z);
	y->y = (a->m12 * b->x) + (a->m22 * b->y) + (a->m32 * b->z);
	y->z = (a->m13 * b->x) + (a->m23 * b->y) + (a->m33 * b->z);
}




//Copy all points inside the ball
static uint32_t v3f32_ball (v3f32 const x[], uint32_t n, v3f32 const * c, v3f32 y[], float r)
{
	uint32_t m = 0;
	for(uint32_t i = 0; i < n; ++i)
	{
		v3f32 d;
		v3f32_sub (&d, x + i, c);
		if (v3f32_norm2 (&d) < (r*r))
		{
			y[m] = x[i];
			m++;
		}
	}
	return m;
}







static void v3f32_meanacc (v3f32 * y, v3f32 const x[], uint32_t n)
{
	ASSERT (n > 0); //Divide by zero protection
	for (uint32_t i = 0; i < n; ++i)
	{
		y->x += x[i].x;
		y->y += x[i].y;
		y->z += x[i].z;
	}
	float s = 1.0f / n;
	y->x *= s;
	y->y *= s;
	y->z *= s;
}





static void v3f32_lerp2 (v3f32 * y, v3f32 const * a, v3f32 * const b, float t)
{
	y->x = (1 - t) * a->x + t * b->x;
	y->y = (1 - t) * a->y + t * b->y;
	y->z = (1 - t) * a->z + t * b->z;
}


static void v4f32_cpy (v4f32 * r, v4f32 const * a)
{
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
	r->w = a->w;
}


static void v4f32_add (v4f32 * r, v4f32 const * a, v4f32 const * b)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
	r->w = a->w + b->w;
}


static void v4f32_acc (v4f32 * r, v4f32 const * a)
{
	r->x += a->x;
	r->y += a->y;
	r->z += a->z;
	r->w += a->w;
}


static void v4f32_sub (v4f32 * r, v4f32 const * a, v4f32 const * b)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
	r->w = a->w - b->w;
}

static void v4f32_subv (v4f32 r[], v4f32 const a[], v4f32 const b[], uint32_t incr, uint32_t inca, uint32_t incb, uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		v4f32_sub (r, a, b);
		r += incr;
		a += inca;
		b += incb;
	}
}


static void v4f32_set1 (v4f32 * r, float b)
{
	r->x = b;
	r->y = b;
	r->z = b;
	r->w = b;
}


static void v4f32_mul (v4f32 * r, v4f32 const * a, float b)
{
	r->x = a->x * b;
	r->y = a->y * b;
	r->z = a->z * b;
	r->w = a->w * b;
}


static void v4f32_div (v4f32 * r, v4f32 const * a, float b)
{
	r->x = a->x / b;
	r->y = a->y / b;
	r->z = a->z / b;
	r->w = a->w / b;
}


static float v4f32_dot (v4f32 const * a, v4f32 const * b)
{
	float sum = 0;
	sum += a->x * b->x;
	sum += a->y * b->y;
	sum += a->z * b->z;
	sum += a->w * b->w;
	return sum;
}


static float v4f32_norm2 (v4f32 const * a)
{
	return v4f32_dot (a, a);
}


static float v4f32_norm (v4f32 const * a)
{
	return sqrtf (v4f32_norm2 (a));
}


static void v4f32_normalize (v4f32 * r, v4f32 const * a)
{
	float l = v4f32_norm (a);
	v4f32_div (r, a, l);
}


static void v4f32_set_xyzw (v4f32 * v, float x, float y, float z, float w)
{
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}




static void v4f32_dotv (v4f32 r[], uint32_t inc_r, v4f32 a[], uint32_t inc_a, v4f32 b[], uint32_t inc_b, uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
	{

		r += inc_r;
		a += inc_a;
		b += inc_b;
	}
}






static void v4f32_m4_macc_unsafe (v4f32 * y, m4f32 const * a, v4f32 * const b)
{
	ASSERT_NEQP (y, b);
	y->x += (a->m11 * b->x) + (a->m12 * b->y) + (a->m13 * b->z) + (a->m14 * b->w);
	y->y += (a->m21 * b->x) + (a->m22 * b->y) + (a->m23 * b->z) + (a->m24 * b->w);
	y->z += (a->m31 * b->x) + (a->m32 * b->y) + (a->m33 * b->z) + (a->m34 * b->w);
	y->w += (a->m41 * b->x) + (a->m42 * b->y) + (a->m43 * b->z) + (a->m44 * b->w);
}

static void v4f32_m4_macc (v4f32 * y, m4f32 const * a, v4f32 * const b)
{
	v4f32 t;
	v4f32_cpy (&t, y);
	t.x += (a->m11 * b->x) + (a->m12 * b->y) + (a->m13 * b->z) + (a->m14 * b->w);
	t.y += (a->m21 * b->x) + (a->m22 * b->y) + (a->m23 * b->z) + (a->m24 * b->w);
	t.z += (a->m31 * b->x) + (a->m32 * b->y) + (a->m33 * b->z) + (a->m34 * b->w);
	t.w += (a->m41 * b->x) + (a->m42 * b->y) + (a->m43 * b->z) + (a->m44 * b->w);
	v4f32_cpy (y, &t);
}



static void v4f32_m4_macct_unsafe (v4f32 * y, m4f32 const * a, v4f32 * const b)
{
	ASSERT_NEQP (y, b);
	y->x += (a->m11 * b->x) + (a->m21 * b->y) + (a->m31 * b->z) + (a->m41 * b->w);
	y->y += (a->m12 * b->x) + (a->m22 * b->y) + (a->m32 * b->z) + (a->m42 * b->w);
	y->z += (a->m13 * b->x) + (a->m23 * b->y) + (a->m33 * b->z) + (a->m43 * b->w);
	y->w += (a->m14 * b->x) + (a->m24 * b->y) + (a->m34 * b->z) + (a->m44 * b->w);
}

static void v4f32_m4_macct (v4f32 * y, m4f32 const * a, v4f32 * const b)
{
	v4f32 t;
	v4f32_cpy (&t, y);
	t.x += (a->m11 * b->x) + (a->m21 * b->y) + (a->m31 * b->z) + (a->m41 * b->w);
	t.y += (a->m12 * b->x) + (a->m22 * b->y) + (a->m32 * b->z) + (a->m42 * b->w);
	t.z += (a->m13 * b->x) + (a->m23 * b->y) + (a->m33 * b->z) + (a->m43 * b->w);
	t.w += (a->m14 * b->x) + (a->m24 * b->y) + (a->m34 * b->z) + (a->m44 * b->w);
	v4f32_cpy (y, &t);
}

static void v4f32_m4_mul_unsafe (v4f32 * y, m4f32 const * a, v4f32 * const b)
{
	ASSERT_NEQP (y, b);
	y->x = (a->m11 * b->x) + (a->m12 * b->y) + (a->m13 * b->z) + (a->m14 * b->w);
	y->y = (a->m21 * b->x) + (a->m22 * b->y) + (a->m23 * b->z) + (a->m24 * b->w);
	y->z = (a->m31 * b->x) + (a->m32 * b->y) + (a->m33 * b->z) + (a->m34 * b->w);
	y->w = (a->m41 * b->x) + (a->m42 * b->y) + (a->m43 * b->z) + (a->m44 * b->w);
}

static void v4f32_m4_mul (v4f32 * y, m4f32 const * a, v4f32 * const b)
{
	v4f32 t;
	t.x = (a->m11 * b->x) + (a->m12 * b->y) + (a->m13 * b->z) + (a->m14 * b->w);
	t.y = (a->m21 * b->x) + (a->m22 * b->y) + (a->m23 * b->z) + (a->m24 * b->w);
	t.z = (a->m31 * b->x) + (a->m32 * b->y) + (a->m33 * b->z) + (a->m34 * b->w);
	t.w = (a->m41 * b->x) + (a->m42 * b->y) + (a->m43 * b->z) + (a->m44 * b->w);
	v4f32_cpy (y, &t);
}


























//Set r = xyzw xyzw xyzw ... n times
static void v4f32_set_xyzw_repeat (uint32_t n, float r [], float x, float y, float z, float w)
{
	while (n--)
	{
		r [0] = x;
		r [1] = y;
		r [2] = z;
		r [3] = w;
		r += 4;
	}
}


static void v4f32_set_w_repeat (uint32_t n, float r [], float w)
{
	while (n--)
	{
		r [3] = w;
		r += 4;
	}
}


static void v4f32_repeat_random (unsigned n, float r [])
{
	uint32_t const dim = 4;
	while (n--)
	{
		vf32_random (dim, r);
		r += dim;
	}
}


static void v4f32_repeat_channel (unsigned n, float r [], unsigned channel, float a)
{
	while (n--)
	{
		r [channel] = a;
		r += 4;
	}
}



static void qf32_cpy (qf32 * r, qf32 const * a)
{
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
	r->w = a->w;
}



static void qf32_identity (qf32 * q)
{
	q->x = 0.0f;
	q->y = 0.0f;
	q->z = 0.0f;
	q->w = 1.0f;
}


static float qf32_norm2 (qf32 * q)
{
	return v4f32_norm2 ((v4f32 *)q);
}


static float qf32_norm (qf32 const * q)
{
	return v4f32_norm ((v4f32 *)q);
}


static void qf32_normalize (qf32 * r, qf32 const * q)
{
	v4f32_normalize ((v4f32 *)r, (v4f32 const *)q);
}


static void qf32_xyza (qf32 * q, float x, float y, float z, float a)
{
	float const c = cosf (a * 0.5f);
	float const s = sinf (a * 0.5f);
	q->x = s * x;
	q->y = s * y;
	q->z = s * z;
	q->w = c;
	qf32_normalize (q, q);
	//vf32_normalize (4, q, q);
	//float n = vf32_norm (4, q);
	//printf ("%f\n", n);
}


static void qf32_axis_angle (qf32 * q, v3f32 const * v, float angle)
{
	qf32_xyza (q, v->x, v->y, v->z, angle);
}


static void qf32_mul1 (qf32 * r, qf32 const * p, qf32 const * q)
{
	ASSERT (r != p);
	ASSERT (r != q);
	r->x = p->w * q->x + p->x * q->w + p->y * q->z - p->z * q->y;
	r->y = p->w * q->y - p->x * q->z + p->y * q->w + p->z * q->x;
	r->z = p->w * q->z + p->x * q->y - p->y * q->x + p->z * q->w;
	r->w = p->w * q->w - p->x * q->x - p->y * q->y - p->z * q->z;
}


/*
static void qf32_mul2 (qf32 * r, qf32 const * p, qf32 const * q)
{
	r->x = p->w * q->x + p->x * q->w;
	r->y = p->w * q->y - p->x * q->z;
	r->z = p->w * q->z + p->x * q->y;
	r->w = p->w * q->w - p->x * q->x;
}
*/


static void qf32_mul (qf32 * r, qf32 const * p, qf32 const * q)
{
	qf32 t;
	qf32_mul1 (&t, p, q);
	qf32_cpy (r, &t);
}



static void qf32_unit_m4 (m4f32 * r, qf32 const * q)
{
	float a = q->w;
	float b = q->x;
	float c = q->y;
	float d = q->z;
	float a2 = a * a;
	float b2 = b * b;
	float c2 = c * c;
	float d2 = d * d;

	r->m11 = a2 + b2 - c2 - d2;
	r->m21 = 2.0f * (b*c + a*d);
	r->m31 = 2.0f * (b*d - a*c);

	r->m12 = 2.0f * (b*c - a*d);
	r->m22 = a2 - b2 + c2 - d2;
	r->m32 = 2.0f * (c*d + a*b);

	r->m13 = 2.0f * (b*d + a*c);
	r->m23 = 2.0f * (c*d - a*b);
	r->m33 = a2 - b2 - c2 + d2;
}


static void qf32_unit_m3 (m3f32 * r, qf32 const * q)
{
	float a = q->w;
	float b = q->x;
	float c = q->y;
	float d = q->z;
	float a2 = a * a;
	float b2 = b * b;
	float c2 = c * c;
	float d2 = d * d;

	r->m11 = a2 + b2 - c2 - d2;
	r->m21 = 2.0f * (b*c + a*d);
	r->m31 = 2.0f * (b*d - a*c);

	r->m12 = 2.0f * (b*c - a*d);
	r->m22 = a2 - b2 + c2 - d2;
	r->m32 = 2.0f * (c*d + a*b);

	r->m13 = 2.0f * (b*d + a*c);
	r->m23 = 2.0f * (c*d - a*b);
	r->m33 = a2 - b2 - c2 + d2;
}


static void qf32_m4 (m4f32 * r, qf32 const * q)
{
	float const l = qf32_norm (q);
	float const s = (l > 0.0f) ? (2.0f / l) : 0.0f;

	float const x = q->x;
	float const y = q->y;
	float const z = q->z;
	float const w = q->w;

	float const xx = s * x * x;
	float const xy = s * x * y;
	float const xz = s * x * z;
	float const xw = s * x * w;

	float const yy = s * y * y;
	float const yz = s * y * z;
	float const yw = s * y * w;

	float const zz = s * z * z;
	float const zw = s * z * w;

	r->m11 = 1.0f - yy - zz;
	r->m22 = 1.0f - xx - zz;
	r->m33 = 1.0f - xx - yy;

	r->m12 = xy - zw;
	r->m21 = xy + zw;

	r->m23 = yz - xw;
	r->m32 = yz + xw;

	r->m31 = xz - yw;
	r->m13 = xz + yw;
}


static void qf32_m3 (m3f32 * r, qf32 const * q)
{
	float const l = qf32_norm (q);
	float const s = (l > 0.0f) ? (2.0f / l) : 0.0f;

	float const x = q->x;
	float const y = q->y;
	float const z = q->z;
	float const w = q->w;

	float const xx = s * x * x;
	float const xy = s * x * y;
	float const xz = s * x * z;
	float const xw = s * x * w;

	float const yy = s * y * y;
	float const yz = s * y * z;
	float const yw = s * y * w;

	float const zz = s * z * z;
	float const zw = s * z * w;

	r->m11 = 1.0f - yy - zz;
	r->m22 = 1.0f - xx - zz;
	r->m33 = 1.0f - xx - yy;

	r->m12 = xy - zw;
	r->m21 = xy + zw;

	r->m23 = yz - xw;
	r->m32 = yz + xw;

	r->m31 = xz - yw;
	r->m13 = xz + yw;
}




/*
https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
void rotate_vector_by_quaternion(const Vector3& v, const Quaternion& q, Vector3& vprime)
{
	// Extract the vector part of the quaternion
	Vector3 u(q.x, q.y, q.z);

	// Extract the scalar part of the quaternion
	float s = q.w;

	// Do the math
	vprime = 2.0f * dot(u, v) * u
		  + (s*s - dot(u, u)) * v
		  + 2.0f * s * cross(u, v);
}

static void qf32_rotate_vector_fixthis (qf32 u, v3f32 y)
{
	v3f32 v;
	v3f32_cpy (&v, y);
	float uv = vf32_dot (3, u, v);
	float ww = u[3] * u[3];
	vf32_set1 (3, y, 0.0f);
	vsf32_macc (3, y, u, 2.0f * uv);
	vsf32_macc (3, y, v, ww - uv);
	v3f32_crossacc_scalar (y, 2.0f * ww, u, v);
}
*/


/*
https://github.com/datenwolf/linmath.h/blob/382ba71905c2c09f10684d19cb5a3fcadf1aba39/linmath.h#L494
Method by Fabian 'ryg' Giessen (of Farbrausch)
t = 2 * cross(q.xyz, v)
v' = v + q.w * t + cross(q.xyz, t)
*/
static void qf32_rotate_vector (qf32 const * q, v3f32 const * v, v3f32 * r)
{
	ASSERT (v != r);
	v3f32 t;
	v3f32 u = {{q->x, q->y, q->z}};
	v3f32_cross (&t, (v3f32 *)q, v);
	v3f32_mul (&t, &t, 2.0f);
	v3f32_cross (&u, (v3f32 *)q, &t);
	v3f32_mul (&t, &t, q->w);
	v3f32_add (r, v, &t);
	v3f32_add (r, r, &u);
}


static void qf32_rotate_v3f32 (qf32 * q, v3f32 * v)
{
	v3f32 r;
	qf32_rotate_vector (q, v, &r);
	v3f32_cpy (v, &r);
}


static void qf32_rotate_v3f32_array (qf32 * q, v3f32 v[], uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		qf32_rotate_v3f32 (q, v + i);
	}
}


static void qf32_rotate1_xyza (qf32 * q, float x, float y, float z, float a)
{
	qf32 u;
	qf32_xyza (&u, x, y, z, a);
	qf32_mul (q, &u, q);
}

static void qf32_rotate2_xyza (qf32 * q, float x, float y, float z, float a)
{
	qf32 u;
	qf32_xyza (&u, x, y, z, a);
	qf32_mul (q, q, &u); // q = q * u
}



// yaw (Z), pitch (Y), roll (X)
static void qf32_ypr (qf32 * q, float yaw, float pitch, float roll)
{
	// Abbreviations for the various angular functions
	float cy = cos(yaw * 0.5);
	float sy = sin(yaw * 0.5);
	float cp = cos(pitch * 0.5);
	float sp = sin(pitch * 0.5);
	float cr = cos(roll * 0.5);
	float sr = sin(roll * 0.5);
	q->x = sr * cp * cy - cr * sp * sy;
	q->y = cr * sp * cy + sr * cp * sy;
	q->z = cr * cp * sy - sr * sp * cy;
	q->w = cr * cp * cy + sr * sp * sy;
}


//http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
static void qf32_from_m4 (qf32 * q, m4f32 * m)
{
	q->w = sqrtf (1.0f + m->m11 + m->m22 + m->m33) / 2.0f;
	float w4 = (4.0f * q->w);
	q->x = (m->m32 - m->m21) / w4;
	q->y = (m->m13 - m->m31) / w4;
	q->z = (m->m21 - m->m12) / w4;
}


//http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
static void qf32_from_m3 (qf32 * q, m3f32 * m)
{
	q->w = sqrtf (1.0f + m->m11 + m->m22 + m->m33) / 2.0f;
	float w4 = (4.0f * q->w);
	q->x = (m->m32 - m->m21) / w4;
	q->y = (m->m13 - m->m31) / w4;
	q->z = (m->m21 - m->m12) / w4;
}
