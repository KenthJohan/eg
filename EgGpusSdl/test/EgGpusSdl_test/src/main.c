
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <EgGpusSdl_test.h>

// Testsuite 'GpuResources'
void GpuResources_setup(void);
void GpuResources_teardown(void);
void GpuResources_test_import(void);
void GpuResources_test_device_add_initializes_handle(void);
void GpuResources_test_texture_add_initializes_handle(void);
void GpuResources_test_invalid_texture_size_disables_entity(void);
void GpuResources_test_vertex_shader_create(void);
void GpuResources_test_fragment_shader_create(void);
void GpuResources_test_graphics_pipeline_create(void);
void GpuResources_test_graphics_pipeline_create_with_position_color_uv(void);

bake_test_case GpuResources_testcases[] = {
    {
        "test_import",
        GpuResources_test_import
    },
    {
        "test_device_add_initializes_handle",
        GpuResources_test_device_add_initializes_handle
    },
    {
        "test_texture_add_initializes_handle",
        GpuResources_test_texture_add_initializes_handle
    },
    {
        "test_invalid_texture_size_disables_entity",
        GpuResources_test_invalid_texture_size_disables_entity
    },
    {
        "test_vertex_shader_create",
        GpuResources_test_vertex_shader_create
    },
    {
        "test_fragment_shader_create",
        GpuResources_test_fragment_shader_create
    },
    {
        "test_graphics_pipeline_create",
        GpuResources_test_graphics_pipeline_create
    },
    {
        "test_graphics_pipeline_create_with_position_color_uv",
        GpuResources_test_graphics_pipeline_create_with_position_color_uv
    }
};

static bake_test_suite suites[] = {
    {
        "GpuResources",
        GpuResources_setup,
        GpuResources_teardown,
        8,
        GpuResources_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgGpusSdl_test", argc, argv, suites, 1);
}
