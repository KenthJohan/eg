#include <egmath_test.h>
#include <egmath.h>
#include <stdio.h>
#include <math.h>

#define EPSILON               1e-5f
#define ASSERT_FLOAT_EQ(a, b) test_assert(fabsf((a) - (b)) < EPSILON)
#define ASSERT_V3_EQ(a, b)               \
	do {                                 \
		ASSERT_FLOAT_EQ((a)[0], (b)[0]); \
		ASSERT_FLOAT_EQ((a)[1], (b)[1]); \
		ASSERT_FLOAT_EQ((a)[2], (b)[2]); \
	} while (0)
#define ASSERT_M3_EQ(a, b)            \
	do {                              \
		ASSERT_V3_EQ((a).c0, (b).c0); \
		ASSERT_V3_EQ((a).c1, (b).c1); \
		ASSERT_V3_EQ((a).c2, (b).c2); \
	} while (0)

void Matrix_setup(void)
{
}

void Matrix_add_m3m3(void)
{
	m3f32 a = {
	.c0 = {1.0f, 2.0f, 3.0f},
	.c1 = {4.0f, 5.0f, 6.0f},
	.c2 = {7.0f, 8.0f, 9.0f}};
	m3f32 b = {
	.c0 = {1.0f, 1.0f, 1.0f},
	.c1 = {2.0f, 2.0f, 2.0f},
	.c2 = {3.0f, 3.0f, 3.0f}};
	m3f32 result;

	m3f32_add(&result, &a, &b);

	m3f32 expected = {
	.c0 = {2.0f, 3.0f, 4.0f},
	.c1 = {6.0f, 7.0f, 8.0f},
	.c2 = {10.0f, 11.0f, 12.0f}};

	ASSERT_M3_EQ(result, expected);
}
