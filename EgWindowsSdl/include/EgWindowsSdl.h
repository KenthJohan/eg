#pragma once
#include <flecs.h>

void EgWindowsSdlImport(ecs_world_t *world);

/* Refresh window and pointer state after SDL events have been pumped. */
void EgWindowsSdlUpdateState(ecs_world_t *world);
