#pragma once
#include <flecs.h>
#include <glslang/Include/glslang_c_interface.h>


typedef struct
{
	ecs_i32_t stage; // 0=vert, 4=frag, ...
} EgGlslangCreate;

typedef struct
{
	uint32_t * words;
	int32_t words_size;
} EgGlslangProgram;

extern ECS_COMPONENT_DECLARE(EgGlslangCreate);
extern ECS_COMPONENT_DECLARE(EgGlslangProgram);

/**
 * Compile GLSL source code to SPIRV bytecode
 * @param stage glslang shader stage (GLSLANG_STAGE_VERTEX, GLSLANG_STAGE_FRAGMENT, etc.)
 * @param shaderSource GLSL source code as null-terminated string
 * @return EgGlslangProgram containing SPIRV words and size, or {NULL, 0} on failure
 */
EgGlslangProgram compileShaderToSPIRV_Vulkan(glslang_stage_t stage, const char *shaderSource);

void EgGlslangImport(ecs_world_t *world);
