
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <egmath_test.h>

// Testsuite 'Quaternion'
void Quaternion_setup(void);
void Quaternion_test1(void);
void Quaternion_test_identity(void);
void Quaternion_test_xyza(void);
void Quaternion_test_normalize(void);
void Quaternion_test_mul_identity(void);
void Quaternion_test_mul_associativity(void);
void Quaternion_test_rotate_vector_identity(void);
void Quaternion_test_rotate_vector_z_90(void);
void Quaternion_test_rotate_vector_x_180(void);
void Quaternion_test_unit_to_m4_identity(void);
void Quaternion_test_unit_to_m3_identity(void);
void Quaternion_test_from_euler_zero(void);
void Quaternion_test_from_euler_roll_90(void);
void Quaternion_test_composed_rotations(void);

bake_test_case Quaternion_testcases[] = {
    {
        "test1",
        Quaternion_test1
    },
    {
        "test_identity",
        Quaternion_test_identity
    },
    {
        "test_xyza",
        Quaternion_test_xyza
    },
    {
        "test_normalize",
        Quaternion_test_normalize
    },
    {
        "test_mul_identity",
        Quaternion_test_mul_identity
    },
    {
        "test_mul_associativity",
        Quaternion_test_mul_associativity
    },
    {
        "test_rotate_vector_identity",
        Quaternion_test_rotate_vector_identity
    },
    {
        "test_rotate_vector_z_90",
        Quaternion_test_rotate_vector_z_90
    },
    {
        "test_rotate_vector_x_180",
        Quaternion_test_rotate_vector_x_180
    },
    {
        "test_unit_to_m4_identity",
        Quaternion_test_unit_to_m4_identity
    },
    {
        "test_unit_to_m3_identity",
        Quaternion_test_unit_to_m3_identity
    },
    {
        "test_from_euler_zero",
        Quaternion_test_from_euler_zero
    },
    {
        "test_from_euler_roll_90",
        Quaternion_test_from_euler_roll_90
    },
    {
        "test_composed_rotations",
        Quaternion_test_composed_rotations
    }
};


static bake_test_suite suites[] = {
    {
        "Quaternion",
        Quaternion_setup,
        NULL,
        14,
        Quaternion_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("egmath_test", argc, argv, suites, 1);
}
