#pragma once

#include <SDL3/SDL_gpu.h>
#include <flecs.h>

#define MAX_ATTRIBUTES 16

void EcsMember_to_SDL_GPUVertexAttribute(SDL_GPUVertexAttribute *attribute, EcsMember const *member);

void EcsMember_to_SDL_GPUVertexAttribute_array(SDL_GPUVertexAttribute attribute[], ecs_world_t *world, ecs_entity_t type);
