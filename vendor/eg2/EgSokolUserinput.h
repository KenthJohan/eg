#pragma once
#include <flecs.h>
#include "sokol_app.h"

void sokol_input_action(const sapp_event* evt, ecs_world_t *world);
void EgSokolUserinputImport(ecs_world_t *world);
