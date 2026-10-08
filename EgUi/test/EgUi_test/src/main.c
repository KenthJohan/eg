
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
void FlowLayout_test_children_flow_right_then_wrap_down(void);
void FlowLayout_test_none_direction_leaves_children_untouched(void);

// Testsuite 'MouseHitTesting'
void MouseHitTesting_setup(void);
void MouseHitTesting_teardown(void);
void MouseHitTesting_test_axis_aligned_hit_and_clear(void);
void MouseHitTesting_test_rotated_scaled_hit(void);
void MouseHitTesting_test_overlapping_rectangles_all_hover(void);

bake_test_case FlowLayout_testcases[] = {
    {
        "test_children_flow_right_then_wrap_down",
        FlowLayout_test_children_flow_right_then_wrap_down
    },
    {
        "test_none_direction_leaves_children_untouched",
        FlowLayout_test_none_direction_leaves_children_untouched
    }
};

bake_test_case MouseHitTesting_testcases[] = {
    {
        "test_axis_aligned_hit_and_clear",
        MouseHitTesting_test_axis_aligned_hit_and_clear
    },
    {
        "test_rotated_scaled_hit",
        MouseHitTesting_test_rotated_scaled_hit
    },
    {
        "test_overlapping_rectangles_all_hover",
        MouseHitTesting_test_overlapping_rectangles_all_hover
    }
};

static bake_test_suite suites[] = {
    {
        "FlowLayout",
        FlowLayout_setup,
        FlowLayout_teardown,
        2,
        FlowLayout_testcases
    },
    {
        "MouseHitTesting",
        MouseHitTesting_setup,
        MouseHitTesting_teardown,
        3,
        MouseHitTesting_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgUi_test", argc, argv, suites, 2);
}
