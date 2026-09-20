
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <EgFs_test.h>

// Testsuite 'TestEgFs'
void TestEgFs_setup(void);
void TestEgFs_teardown(void);
void TestEgFs_test_path_type_schemes(void);
void TestEgFs_test_path1_script_function(void);

bake_test_case TestEgFs_testcases[] = {
    {
        "test_path_type_schemes",
        TestEgFs_test_path_type_schemes
    },
    {
        "test_path1_script_function",
        TestEgFs_test_path1_script_function
    }
};

static bake_test_suite suites[] = {
    {
        "TestEgFs",
        TestEgFs_setup,
        TestEgFs_teardown,
        2,
        TestEgFs_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgFs_test", argc, argv, suites, 1);
}
