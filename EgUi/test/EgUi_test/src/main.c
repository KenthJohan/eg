
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
void FlowLayout_test_parent_clearance_is_signed(void);
void FlowLayout_test_oversized_child_is_skipped_and_retried(void);
void FlowLayout_test_manually_disabled_child_stays_disabled(void);

// Testsuite 'MouseHitTesting'
void MouseHitTesting_setup(void);
void MouseHitTesting_teardown(void);
void MouseHitTesting_test_axis_aligned_hit_and_clear(void);
void MouseHitTesting_test_rotated_scaled_hit(void);
void MouseHitTesting_test_overlapping_rectangles_all_hover(void);
void MouseHitTesting_test_disabled_ancestor_clears_hover(void);

// Testsuite 'Resizable'
void Resizable_setup(void);
void Resizable_teardown(void);
void Resizable_test_right_edge_stops_at_parent_boundary(void);
void Resizable_test_disabled_entity_cancels_drag(void);

bake_test_case FlowLayout_testcases[] = {
    {
        "test_children_flow_right_then_wrap_down",
        FlowLayout_test_children_flow_right_then_wrap_down
    },
    {
        "test_none_direction_leaves_children_untouched",
        FlowLayout_test_none_direction_leaves_children_untouched
    },
    {
        "test_parent_clearance_is_signed",
        FlowLayout_test_parent_clearance_is_signed
    },
    {
        "test_oversized_child_is_skipped_and_retried",
        FlowLayout_test_oversized_child_is_skipped_and_retried
    },
    {
        "test_manually_disabled_child_stays_disabled",
        FlowLayout_test_manually_disabled_child_stays_disabled
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
    },
    {
        "test_disabled_ancestor_clears_hover",
        MouseHitTesting_test_disabled_ancestor_clears_hover
    }
};

bake_test_case Resizable_testcases[] = {
    {
        "test_right_edge_stops_at_parent_boundary",
        Resizable_test_right_edge_stops_at_parent_boundary
    },
    {
        "test_disabled_entity_cancels_drag",
        Resizable_test_disabled_entity_cancels_drag
    }
};

static bake_test_suite suites[] = {
    {
        "FlowLayout",
        FlowLayout_setup,
        FlowLayout_teardown,
        5,
        FlowLayout_testcases
    },
    {
        "MouseHitTesting",
        MouseHitTesting_setup,
        MouseHitTesting_teardown,
        4,
        MouseHitTesting_testcases
    },
    {
        "Resizable",
        Resizable_setup,
        Resizable_teardown,
        2,
        Resizable_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgUi_test", argc, argv, suites, 3);
}
