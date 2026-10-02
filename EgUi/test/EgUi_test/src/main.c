
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

// Testsuite 'TableLayout'
void TableLayout_setup(void);
void TableLayout_teardown(void);
void TableLayout_test_cells_size_to_largest_child_and_position(void);
void TableLayout_test_empty_table_has_no_space(void);
void TableLayout_test_sparse_cells_leave_empty_columns(void);
void TableLayout_test_relayout_after_child_resize(void);

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

bake_test_case TableLayout_testcases[] = {
    {
        "test_cells_size_to_largest_child_and_position",
        TableLayout_test_cells_size_to_largest_child_and_position
    },
    {
        "test_empty_table_has_no_space",
        TableLayout_test_empty_table_has_no_space
    },
    {
        "test_sparse_cells_leave_empty_columns",
        TableLayout_test_sparse_cells_leave_empty_columns
    },
    {
        "test_relayout_after_child_resize",
        TableLayout_test_relayout_after_child_resize
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
        "TableLayout",
        TableLayout_setup,
        TableLayout_teardown,
        4,
        TableLayout_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgUi_test", argc, argv, suites, 2);
}
