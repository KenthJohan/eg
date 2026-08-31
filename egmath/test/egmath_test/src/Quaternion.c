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
#define ASSERT_V4_EQ(a, b)               \
	do {                                 \
		ASSERT_FLOAT_EQ((a)[0], (b)[0]); \
		ASSERT_FLOAT_EQ((a)[1], (b)[1]); \
		ASSERT_FLOAT_EQ((a)[2], (b)[2]); \
		ASSERT_FLOAT_EQ((a)[3], (b)[3]); \
	} while (0)

void Quaternion_test_identity(void)
{
	float q[4] = QF32_IDENTITY;

	// Identity quaternion should be (0, 0, 0, 1)
	ASSERT_V4_EQ(q, ((float[]){0, 0, 0, 1}));
}

void Quaternion_test_xyza(void)
{
	float q[4];

	// Rotation of 90 degrees around Z axis
	qf32_xyza(q, 0.0f, 0.0f, 1.0f, M_PI / 2.0f);

	// Expected: sin(45°) * (0, 0, 1), cos(45°)
	float s45 = sinf(M_PI / 4.0f);
	float c45 = cosf(M_PI / 4.0f);

	ASSERT_FLOAT_EQ(q[0], 0.0f);
	ASSERT_FLOAT_EQ(q[1], 0.0f);
	ASSERT_FLOAT_EQ(q[2], s45);
	ASSERT_FLOAT_EQ(q[3], c45);
}

void Quaternion_test_normalize(void)
{
	float q[4] = {1.0f, 2.0f, 3.0f, 4.0f};
	float r[4];

	qf32_normalize(r, q, 0.0f);

	// Magnitude should be close to 1
	float mag_sq = V4_DOT(r, r);
	ASSERT_FLOAT_EQ(mag_sq, 1.0f);
}

void Quaternion_test_mul_identity(void)
{
	float identity[4] = QF32_IDENTITY;
	float q[4]        = {0.5f, 0.3f, 0.2f, 0.8f};
	float result[4];

	qf32_mul(result, identity, q);

	ASSERT_V4_EQ(result, q);
}

void Quaternion_test_mul_associativity(void)
{
	float q1[4] = {0.1f, 0.2f, 0.3f, 0.9f};
	float q2[4] = {0.4f, 0.1f, 0.2f, 0.8f};
	float q3[4] = {0.2f, 0.3f, 0.4f, 0.7f};

	float temp1[4], result1[4];
	float temp2[4], result2[4];

	// (q1 * q2) * q3
	qf32_mul(temp1, q1, q2);
	qf32_mul(result1, temp1, q3);

	// q1 * (q2 * q3)
	qf32_mul(temp2, q2, q3);
	qf32_mul(result2, q1, temp2);

	ASSERT_V4_EQ(result1, result2);
}

void Quaternion_test_rotate_vector_identity(void)
{
	float identity[4] = QF32_IDENTITY;
	float v[3]        = {1.0f, 2.0f, 3.0f};
	float result[3];

	qf32_rotate_vector(identity, v, result);

	// Identity rotation should not change the vector
	ASSERT_V3_EQ(result, v);
}

void Quaternion_test_rotate_vector_z_90(void)
{
	float q[4];
	qf32_xyza(q, 0.0f, 0.0f, 1.0f, M_PI / 2.0f);

	float v[3] = {1.0f, 0.0f, 0.0f};
	float result[3];

	qf32_rotate_vector(q, v, result);

	// (1, 0, 0) rotated 90° around Z should be approximately (0, 1, 0)
	ASSERT_FLOAT_EQ(result[0], 0.0f);
	ASSERT_FLOAT_EQ(result[1], 1.0f);
	ASSERT_FLOAT_EQ(result[2], 0.0f);
}

void Quaternion_test_rotate_vector_x_180(void)
{
	float q[4];
	qf32_xyza(q, 1.0f, 0.0f, 0.0f, M_PI);

	float v[3] = {0.0f, 1.0f, 0.0f};
	float result[3];

	qf32_rotate_vector(q, v, result);

	// (0, 1, 0) rotated 180° around X should be approximately (0, -1, 0)
	ASSERT_FLOAT_EQ(result[0], 0.0f);
	ASSERT_FLOAT_EQ(result[1], -1.0f);
	ASSERT_FLOAT_EQ(result[2], 0.0f);
}

void Quaternion_test_unit_to_m4_identity(void)
{
	float identity[4] = QF32_IDENTITY;
	m4f32 m;

	qf32_unit_to_m4(identity, &m);

	// Should be identity matrix
	m4f32 expected = M4_IDENTITY;

	ASSERT_V4_EQ(m.c0, expected.c0);
	ASSERT_V4_EQ(m.c1, expected.c1);
	ASSERT_V4_EQ(m.c2, expected.c2);
	ASSERT_V4_EQ(m.c3, expected.c3);
}

void Quaternion_test_unit_to_m3_identity(void)
{
	float identity[4] = QF32_IDENTITY;
	m3f32 m;

	qf32_unit_to_m3(identity, &m);

	// Should be identity matrix
	m3f32 expected = M3_IDENTITY;

	ASSERT_V3_EQ(m.c0, expected.c0);
	ASSERT_V3_EQ(m.c1, expected.c1);
	ASSERT_V3_EQ(m.c2, expected.c2);
}

void Quaternion_test_from_euler_zero(void)
{
	float q[4];
	qf32_from_euler(q, 0.0f, 0.0f, 0.0f);

	// Zero euler angles should give identity quaternion
	float identity[4] = QF32_IDENTITY;
	ASSERT_V4_EQ(q, identity);
}

void Quaternion_test_from_euler_roll_90(void)
{
	float q[4];
	qf32_from_euler(q, 0.0f, 0.0f, M_PI / 2.0f);

	// 90 degree roll should rotate around X
	// Check normalized magnitude
	float mag_sq = V4_DOT(q, q);
	ASSERT_FLOAT_EQ(mag_sq, 1.0f);

	// Verify by rotating (0, 1, 0) should give approximately (0, 0, 1)
	float v[3] = {0.0f, 1.0f, 0.0f};
	float result[3];
	qf32_rotate_vector(q, v, result);

	ASSERT_FLOAT_EQ(result[0], 0.0f);
	ASSERT_FLOAT_EQ(result[1], 0.0f);
	ASSERT_FLOAT_EQ(result[2], 1.0f);
}

void Quaternion_test_composed_rotations(void)
{
	float q1[4], q2[4], composed[4];

	// 90 degrees around Z
	qf32_xyza(q1, 0.0f, 0.0f, 1.0f, M_PI / 2.0f);
	// 90 degrees around X
	qf32_xyza(q2, 1.0f, 0.0f, 0.0f, M_PI / 2.0f);

	// Compose rotations
	qf32_mul(composed, q1, q2);

	// Apply composed rotation to (0, 1, 0)
	float v[3] = {0.0f, 1.0f, 0.0f};
	float result[3];
	qf32_rotate_vector(composed, v, result);

	// Apply individual rotations separately and verify
	float temp[3];
	float expected[3];
	qf32_rotate_vector(q2, v, temp);
	qf32_rotate_vector(q1, temp, expected);
	ASSERT_V3_EQ(result, expected);
}

void Quaternion_setup(void) {
}

