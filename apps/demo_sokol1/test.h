#pragma once

#include "flecs.h"

void frame(ecs_world_t *world);

void init(ecs_world_t *world);

void cleanup(ecs_world_t *world);

void sokol_input_action(const sapp_event* evt, ecs_world_t *world);
