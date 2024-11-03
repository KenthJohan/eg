#include "shader_spirv.h"
#include <stdio.h>
#include <stdint.h>
#include <flecs.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <SDL3/SDL_gpu.h>

static char *static_load_from_file(const char *filename, size_t * length)
{
	FILE *file;
	char *content = NULL;
	int32_t bytes;
	size_t size;

	/* Open file for reading */
	ecs_os_fopen(&file, filename, "r");
	if (!file) {
		ecs_err("%s (%s)", ecs_os_strerror(errno), filename);
		goto error;
	}

	/* Determine file size */
	fseek(file, 0, SEEK_END);
	bytes = (int32_t)ftell(file);
	if (bytes == -1) {
		goto error;
	}
	*length = bytes;
	fseek(file, 0, SEEK_SET);

	/* Load contents in memory */
	content = ecs_os_malloc(bytes + 1);
	size = (size_t)bytes;
	if (!(size = fread(content, 1, size, file)) && bytes) {
		ecs_err("%s: read zero bytes instead of %d", filename, size);
		ecs_os_free(content);
		content = NULL;
		goto error;
	} else {
		content[size] = '\0';
	}

	fclose(file);

	return content;
error:
	ecs_os_free(content);
	return NULL;
}

static char *static_popen(const char *namepart, const char *type, char const * ofile)
{
	FILE *f = NULL;
	char buffer[128];
	// int len = snprintf(buffer, sizeof(buffer), "glslangValidator %s.%s.glsl -V -S %s -o %s.%s.spv --quiet", namepart, type, type, namepart, type);
	int len = snprintf(buffer, sizeof(buffer), "glslangValidator %s.%s.glsl -V -S %s -o %s", namepart, type, type, ofile);
	if (len < 0) {
		return NULL;
	}
	if ((size_t)len > sizeof(buffer)) {
		return NULL;
	}
	printf("popen: %s\n", buffer);
	f = popen(buffer, "r");
	if (f == NULL) {
		return NULL;
	}
	static int line = 0;
	while (1) {
		char *r = fgets(buffer, sizeof(buffer) - 1, f);
		if (r == NULL) {
			break;
		}
		printf("line %04i: %s", line++, buffer);
	}
	int status = pclose(f);
	if (WIFEXITED(status)) {
		int exit_code = WEXITSTATUS(status);
		printf("\nSubprocess exited with code: %d\n", exit_code);
	} else {
		printf("\nSubprocess did not exit normally.\n");
	}
	return NULL;
}





SDL_GPUShader * shader_spirv_compile(SDL_GPUDevice *device, const char *namepart, SDL_GPUShaderStage stage)
{
	char ofilepath[128];
	char const * type = NULL;
	switch (stage)
	{
	case SDL_GPU_SHADERSTAGE_VERTEX:
		type = "vert";
		break;
	case SDL_GPU_SHADERSTAGE_FRAGMENT:
		type = "frag";
		break;
	default:
		break;
	}
	int len = snprintf(ofilepath, sizeof(ofilepath), "%s.%s.spv", namepart, type);
	static_popen(namepart, type, ofilepath);
	size_t length = 0;
	char const *code = static_load_from_file(ofilepath, &length);

	SDL_GPUShaderCreateInfo createinfo;
	createinfo.num_samplers = 0;
	createinfo.num_storage_buffers = 0;
	createinfo.num_storage_textures = 0;
	createinfo.num_uniform_buffers = (stage == SDL_GPU_SHADERSTAGE_VERTEX);
	createinfo.props = 0;

	SDL_GPUShaderFormat format = SDL_GetGPUShaderFormats(device);
	if (format != SDL_GPU_SHADERFORMAT_SPIRV) {
		fprintf(stderr, "Format is not SDL_GPU_SHADERFORMAT_SPIRV");
		return NULL;
	}
	createinfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
	createinfo.code = code;
	createinfo.code_size = length;
	createinfo.entrypoint = "main";
	createinfo.stage = stage;
	SDL_GPUShader * s = SDL_CreateGPUShader(device, &createinfo);

	return s;
}







