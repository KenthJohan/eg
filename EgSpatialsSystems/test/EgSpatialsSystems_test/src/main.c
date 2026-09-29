
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <bake_test.h>

// Testsuite 'Transform'
void Transform_setup(void);
void Transform_teardown(void);
void Transform_test_positive_z_rotation_moves_child_toward_positive_y(void);

bake_test_case Transform_testcases[] = {
    {
        "test_positive_z_rotation_moves_child_toward_positive_y",
        Transform_test_positive_z_rotation_moves_child_toward_positive_y
    }
};

static bake_test_suite suites[] = {
    {
        "Transform",
        Transform_setup,
        Transform_teardown,
        1,
        Transform_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgSpatialsSystems_test", argc, argv, suites, 1);
}
