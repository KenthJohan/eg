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


float v3f32_intersect_cylinder(float const v[3], float const l0[3], float const c[3], float const n[3], float r)
{
	float b2_4ac;
	return b2_4ac;
}


/*
https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html

*/