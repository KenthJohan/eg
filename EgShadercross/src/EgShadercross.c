/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/

#include "EgShadercross.h"
#include "EgFs.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include <ecsx.h>
#include <egmisc.h>



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
