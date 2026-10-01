
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <bake_test.h>

// Testsuite 'FlowLayout'
void FlowLayout_setup(void);
void FlowLayout_teardown(void);
void FlowLayout_test_horizontal_flow_applies_order_padding_and_alignment(void);
void FlowLayout_test_rectangle_dimensions_supply_intrinsic_size(void);

bake_test_case FlowLayout_testcases[] = {
    {
        "test_horizontal_flow_applies_order_padding_and_alignment",
        FlowLayout_test_horizontal_flow_applies_order_padding_and_alignment
    },
    {
        "test_rectangle_dimensions_supply_intrinsic_size",
        FlowLayout_test_rectangle_dimensions_supply_intrinsic_size
    }
};

static bake_test_suite suites[] = {
    {
        "FlowLayout",
        FlowLayout_setup,
        FlowLayout_teardown,
        2,
        FlowLayout_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgUi_test", argc, argv, suites, 1);
}
