
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <EgGpusSdl_test.h>

// Testsuite 'GpuResourcesDevice'
void GpuResourcesDevice_setup(void);
void GpuResourcesDevice_teardown(void);
void GpuResourcesDevice_test_import(void);
void GpuResourcesDevice_test_device_add_initializes_handle(void);

// Testsuite 'GpuResourcesTexture'
void GpuResourcesTexture_setup(void);
void GpuResourcesTexture_teardown(void);
void GpuResourcesTexture_test_texture_add_initializes_handle(void);
void GpuResourcesTexture_test_invalid_texture_size_disables_entity(void);

// Testsuite 'GpuResourcesShader'
void GpuResourcesShader_setup(void);
void GpuResourcesShader_teardown(void);
void GpuResourcesShader_test_vertex_shader_create(void);
void GpuResourcesShader_test_fragment_shader_create(void);

// Testsuite 'GpuResourcesPipeline'
void GpuResourcesPipeline_setup(void);
void GpuResourcesPipeline_teardown(void);
void GpuResourcesPipeline_test_graphics_pipeline_create(void);
void GpuResourcesPipeline_test_graphics_pipeline_create_with_position_color_uv(void);

bake_test_case GpuResourcesDevice_testcases[] = {
    {
        "test_import",
        GpuResourcesDevice_test_import
    },
    {
        "test_device_add_initializes_handle",
        GpuResourcesDevice_test_device_add_initializes_handle
    }
};

bake_test_case GpuResourcesTexture_testcases[] = {
    {
        "test_texture_add_initializes_handle",
        GpuResourcesTexture_test_texture_add_initializes_handle
    },
    {
        "test_invalid_texture_size_disables_entity",
        GpuResourcesTexture_test_invalid_texture_size_disables_entity
    }
};

bake_test_case GpuResourcesShader_testcases[] = {
    {
        "test_vertex_shader_create",
        GpuResourcesShader_test_vertex_shader_create
    },
    {
        "test_fragment_shader_create",
        GpuResourcesShader_test_fragment_shader_create
    }
};

bake_test_case GpuResourcesPipeline_testcases[] = {
    {
        "test_graphics_pipeline_create",
        GpuResourcesPipeline_test_graphics_pipeline_create
    },
    {
        "test_graphics_pipeline_create_with_position_color_uv",
        GpuResourcesPipeline_test_graphics_pipeline_create_with_position_color_uv
    }
};

static bake_test_suite suites[] = {
    {
        "GpuResourcesDevice",
        GpuResourcesDevice_setup,
        GpuResourcesDevice_teardown,
        2,
        GpuResourcesDevice_testcases
    },
    {
        "GpuResourcesTexture",
        GpuResourcesTexture_setup,
        GpuResourcesTexture_teardown,
        2,
        GpuResourcesTexture_testcases
    },
    {
        "GpuResourcesShader",
        GpuResourcesShader_setup,
        GpuResourcesShader_teardown,
        2,
        GpuResourcesShader_testcases
    },
    {
        "GpuResourcesPipeline",
        GpuResourcesPipeline_setup,
        GpuResourcesPipeline_teardown,
        2,
        GpuResourcesPipeline_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("EgGpusSdl_test", argc, argv, suites, 4);
}
