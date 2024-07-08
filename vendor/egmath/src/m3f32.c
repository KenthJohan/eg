#include "egmath.h"

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

void m3f32_transpose(m3f32 *x)
{
	SWAP(float, x->c0[1], x->c1[0]);
	SWAP(float, x->c0[2], x->c2[0]);
	SWAP(float, x->c1[2], x->c2[1]);
}