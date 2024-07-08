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
