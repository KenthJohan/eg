
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <EgFs_test.h>

// Testsuite 'EgFs'
void EgFs_setup(void);
void EgFs_teardown(void);
void EgFs_test_path_type_schemes(void);
void EgFs_test_path1_script_function(void);

bake_test_case EgFs_testcases[] = {
    {
        "test_path_type_schemes",
        EgFs_test_path_type_schemes
    },
    {
        "test_path1_script_function",
        EgFs_test_path1_script_function
    }
};

static bake_test_suite suites[] = {
    {
        "EgFs",
        EgFs_setup,
        EgFs_teardown,
        2,
        EgFs_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgFs_test", argc, argv, suites, 1);
}
