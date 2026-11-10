
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <bake_test.h>

// Testsuite 'Verts'
void Verts_setup(void);
void Verts_teardown(void);
void Verts_test_attributes_and_append(void);
void Verts_test_quad_geometry(void);
void Verts_test_curved_geometry(void);
void Verts_test_shape_counts_and_degenerate_inputs(void);
void Verts_test_pixel_scale(void);
void Verts_test_text(void);
void Verts_test_collectors(void);

bake_test_case Verts_testcases[] = {
    {
        "test_attributes_and_append",
        Verts_test_attributes_and_append
    },
    {
        "test_quad_geometry",
        Verts_test_quad_geometry
    },
    {
        "test_curved_geometry",
        Verts_test_curved_geometry
    },
    {
        "test_shape_counts_and_degenerate_inputs",
        Verts_test_shape_counts_and_degenerate_inputs
    },
    {
        "test_pixel_scale",
        Verts_test_pixel_scale
    },
    {
        "test_text",
        Verts_test_text
    },
    {
        "test_collectors",
        Verts_test_collectors
    }
};

static bake_test_suite suites[] = {
    {
        "Verts",
        Verts_setup,
        Verts_teardown,
        7,
        Verts_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgDraws_test", argc, argv, suites, 1);
}
