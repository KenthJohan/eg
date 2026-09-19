
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <EgSpirv_test.h>

// Testsuite 'Spirv'
void Spirv_setup(void);
void Spirv_teardown(void);
void Spirv_test_import(void);
void Spirv_test_reflect_vertex_inputs(void);
void Spirv_test_invalid_shader_path_disables_entity(void);
void Spirv_test_wrong_stage_disables_entity(void);

bake_test_case Spirv_testcases[] = {
    {
        "test_import",
        Spirv_test_import
    },
    {
        "test_reflect_vertex_inputs",
        Spirv_test_reflect_vertex_inputs
    },
    {
        "test_invalid_shader_path_disables_entity",
        Spirv_test_invalid_shader_path_disables_entity
    },
    {
        "test_wrong_stage_disables_entity",
        Spirv_test_wrong_stage_disables_entity
    }
};

static bake_test_suite suites[] = {
    {
        "Spirv",
        Spirv_setup,
        Spirv_teardown,
        4,
        Spirv_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgSpirv_test", argc, argv, suites, 1);
}
