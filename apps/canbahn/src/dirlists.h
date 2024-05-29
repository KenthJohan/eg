#pragma once
#include <flecs.h>


int dirlists_get(void);

int ecs_plecs_from_dir(ecs_world_t * world, char const * dirpath);