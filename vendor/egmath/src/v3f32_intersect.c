#include "egmath.h"


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
float v3f32_intersect_cylinder_b2_4ac(float const v[3], float const h[3], float const w[3])
{
	float vv = V3_DOT(v, v);
	float vh = V3_DOT(v, h);
	float vw = V3_DOT(v, w);
	float wh = V3_DOT(w, h);
	float ww = V3_DOT(w, w);
	float a = vv - (vh * vh);
	float b = 2.0 * (vw - (vh * wh));
	float c = ww - (wh * wh) - 1.0;
	float b2_4ac = (b * b) - (4.0 * a * c);
	return b2_4ac;
}

/**
 * @brief Convert fs_readfile error to string
 *
 * @param v Vector that defines the line direction in world space
 * @param l0 Point on the line in world space
 * @param c Center of the elliptic cylinder in world space
 * @param h Elliptic cylinder height vector (along local z axis)
 * @param t The transform matrix is the result of the composition of the matrices that describe translation, rotation and scale of the scaled cylinder.
 * @return 
 */
float v3f32_intersect_cylinder1(float const v[3], float const l[3], float const c[3], float const h[3], m4f32 const * t)
{
	float tv[4] = {v[0], v[1], v[2], 0.0};
	float tl[4] = {l[0], l[1], l[2], 1.0};
	float tc[4] = {c[0], c[1], c[2], 1.0};
	m4f32_mulv(t, tl, tl);
	m4f32_mulv(t, tc, tc);
	m4f32_mulv(t, tv, tv);
	float tw[3];
	tw[0] = tl[0] - tc[0];
	tw[1] = tl[1] - tc[1];
	tw[2] = tl[2] - tc[2];
	float b2_4ac = v3f32_intersect_cylinder_b2_4ac(tv, h, tw);
	return b2_4ac;
}


/**
 * @brief Convert fs_readfile error to string
 *
 * @param v Vector that defines the line direction in world space
 * @param l0 Point on the line in world space
 * @param c Center of the elliptic cylinder in world space
 * @param h Elliptic cylinder height vector (along local z axis)
 * @param t The transform matrix is the result of the composition of the matrices that describe translation, rotation and scale of the scaled cylinder.
 * @return 
 */
float v3f32_intersect_cylinder2(float const v[3], float const l[3], float const c[3], float const h[3], m4f32 const * t)
{
	float tv[4] = {v[0], v[1], v[2], 0.0};
	float tl[4] = {l[0], l[1], l[2], 1.0};
	float tc[4] = {c[0], c[1], c[2], 1.0};
	m4f32_mulv(t, tl, tl);
	m4f32_mulv(t, tc, tc);
	m4f32_mulv(t, tv, tv);
	float tw[3];
	tw[0] = tl[0] - tc[0];
	tw[1] = tl[1] - tc[1];
	tw[2] = tl[2] - tc[2];
	float b2_4ac = v3f32_intersect_cylinder_b2_4ac(tv, h, tw);
	return b2_4ac;
}


/**
 * @brief Convert fs_readfile error to string
 *
 * @param v Vector that defines the line direction in world space
 * @param l0 Point on the line in world space
 * @param c Center of the elliptic cylinder in world space
 * @param h Elliptic cylinder height vector (along local z axis)
 * @param t The transform matrix is the result of the composition of the matrices that describe translation, rotation and scale of the scaled cylinder.
 * @return 
 */
float v3f32_intersect_cylinder(float const v[3], float const l[3], float const c[3], float const h[3], float const q[4])
{
	m3f32 r;
	qf32_unit_to_m3(q, &r);
	float tv[3] = {v[0], v[1], v[2]};
	float tw[3];
	tw[0] = l[0] - c[0];
	tw[1] = l[1] - c[1];
	tw[2] = l[2] - c[2];
	m3f32_mulv(&r, tv, tv);
	m3f32_mulv(&r, tw, tw);
	float b2_4ac = v3f32_intersect_cylinder_b2_4ac(tv, h, tw);
	return b2_4ac;
}




/*
https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html

*/