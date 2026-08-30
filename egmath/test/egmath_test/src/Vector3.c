#include <egmath_test.h>
#include <egmath.h>
#include <stdio.h>
#include <math.h>

#define EPSILON 1e-5f
#define ASSERT_FLOAT_EQ(a, b) test_assert(fabsf((a) - (b)) < EPSILON)
#define ASSERT_V3_EQ(a, b) do { \
	ASSERT_FLOAT_EQ((a)[0], (b)[0]); \
	ASSERT_FLOAT_EQ((a)[1], (b)[1]); \
	ASSERT_FLOAT_EQ((a)[2], (b)[2]); \
} while (0)

// Test vector construction from components
static void test_xyz(void)
{
	float v[3];
	v3f32_xyz(v, 1.0f, 2.0f, 3.0f);

	ASSERT_V3_EQ(v, ((float[]){1.0f, 2.0f, 3.0f}));

	printf("✓ Vector3 xyz construction test passed\n");
}

// Test scalar multiplication
static void test_mul(void)
{
	float a[3] = {1.0f, -2.0f, 3.0f};
	float r[3];

	v3f32_mul(r, a, 2.0f);

	ASSERT_V3_EQ(r, ((float[]){2.0f, -4.0f, 6.0f}));

	printf("✓ Vector3 scalar multiplication test passed\n");
}

// Test vector addition
static void test_add(void)
{
	float a[3] = {1.0f, 2.0f, 3.0f};
	float b[3] = {4.0f, -1.0f, 0.5f};
	float r[3];

	v3f32_add(r, a, b);

	ASSERT_V3_EQ(r, ((float[]){5.0f, 1.0f, 3.5f}));

	printf("✓ Vector3 addition test passed\n");
}

// Test cross product of orthogonal unit vectors
static void test_cross(void)
{
	int a[3] = {1, 0, 0};
	int b[3] = {0, 1, 0};
	int r[3];

	v3f32_cross(a, b, r);

	test_assert(r[0] == 0);
	test_assert(r[1] == 0);
	test_assert(r[2] == 1);

	printf("✓ Vector3 cross product test passed\n");
}

// Test euclidean distance between two points
static void test_distance(void)
{
	float a[3] = {0.0f, 0.0f, 0.0f};
	float b[3] = {3.0f, 4.0f, 0.0f};

	float d = v3f32_distance(a, b);

	ASSERT_FLOAT_EQ(d, 5.0f);

	printf("✓ Vector3 distance test passed\n");
}

// Test squared euclidean distance between two points
static void test_distance2(void)
{
	float a[3] = {0.0f, 0.0f, 0.0f};
	float b[3] = {3.0f, 4.0f, 0.0f};

	float d2 = v3f32_distance2(a, b);

	ASSERT_FLOAT_EQ(d2, 25.0f);

	printf("✓ Vector3 squared distance test passed\n");
}

// Test normalization produces a unit vector in the same direction
static void test_normalize(void)
{
	float v[3] = {3.0f, 0.0f, 4.0f};
	float r[3];

	v3f32_normalize(r, v, 0.0f);

	float mag_sq = V3_DOT(r, r);
	ASSERT_FLOAT_EQ(mag_sq, 1.0f);
	ASSERT_V3_EQ(r, ((float[]){0.6f, 0.0f, 0.8f}));

	printf("✓ Vector3 normalize test passed\n");
}

// Test projection of a vector onto a normalized basis vector
static void test_proj(void)
{
	float bnorm[3] = {1.0f, 0.0f, 0.0f};
	float a[3]     = {3.0f, 4.0f, 0.0f};
	float r[3];

	v3f32_proj(bnorm, a, r);

	ASSERT_V3_EQ(r, ((float[]){3.0f, 0.0f, 0.0f}));

	printf("✓ Vector3 projection test passed\n");
}

// Test orthogonal projection (component perpendicular to the basis vector)
static void test_oproj(void)
{
	float bnorm[3] = {1.0f, 0.0f, 0.0f};
	float a[3]     = {3.0f, 4.0f, 0.0f};
	float r[3];

	v3f32_oproj(bnorm, a, r);

	ASSERT_V3_EQ(r, ((float[]){0.0f, 4.0f, 0.0f}));

	printf("✓ Vector3 orthogonal projection test passed\n");
}

// Test shortest distance between two skew lines
static void test_l1l2_distance(void)
{
	float p1[3] = {0.0f, 0.0f, 0.0f};
	float v1[3] = {1.0f, 0.0f, 0.0f};
	float p2[3] = {0.0f, 0.0f, 1.0f};
	float v2[3] = {0.0f, 1.0f, 0.0f};

	float d = v3f32_l1l2_distance(p1, v1, p2, v2);

	// v3f32_l1l2_distance normalizes with a fixed internal epsilon, so allow a looser tolerance here.
	test_assert(fabsf(d - 1.0f) < 1e-3f);

	printf("✓ Vector3 line-to-line distance test passed\n");
}

// Test distance from a point to a plane
static void test_plane_point_line_distance(void)
{
	float v[3] = {0.0f, 0.0f, -1.0f};
	float l0[3] = {0.0f, 0.0f, 5.0f};
	float c[3]  = {0.0f, 0.0f, 0.0f};
	float n[3]  = {0.0f, 0.0f, 1.0f};

	float d = v3f32_plane_point_line_distance(v, l0, c, n);

	ASSERT_FLOAT_EQ(d, 0.0f);

	printf("✓ Vector3 plane-point-line distance test passed\n");
}

// Setup function (called once before tests)
void Vector3_setup(void) {
	printf("\n=== Running Vector3 Tests ===\n");
}

void Vector3_test_xyz(void) {
	test_xyz();
}

void Vector3_test_mul(void) {
	test_mul();
}

void Vector3_test_add(void) {
	test_add();
}

void Vector3_test_cross(void) {
	test_cross();
}

void Vector3_test_distance(void) {
	test_distance();
}

void Vector3_test_distance2(void) {
	test_distance2();
}

void Vector3_test_normalize(void) {
	test_normalize();
}

void Vector3_test_proj(void) {
	test_proj();
}

void Vector3_test_oproj(void) {
	test_oproj();
}

void Vector3_test_l1l2_distance(void) {
	test_l1l2_distance();
}

void Vector3_test_plane_point_line_distance(void) {
	test_plane_point_line_distance();
}
