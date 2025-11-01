#include "EgShadercross.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <stdio.h>
#include <ecsx.h>

void EgShadercrossImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgShadercross);
	ecs_set_name_prefix(world, "EgShadercross");

	bool success = SDL_ShaderCross_Init();
	if (!success) {
		ecs_err("Failed to initialize SDL_ShaderCross");
		return;
	}
}
