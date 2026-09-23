#pragma once

#include <SDL3/SDL_gpu.h>
#include <flecs.h>

#define MAX_ATTRIBUTES 16

void EcsMember_to_SDL_GPUVertexAttribute(SDL_GPUVertexAttribute *attribute, ecs_world_t *world, ecs_entity_t entity);

uint32_t EcsMember_to_SDL_GPUVertexAttribute_array(SDL_GPUVertexAttribute attribute[], ecs_world_t *world, ecs_entity_t type);

// Converts a sample count (1, 2, 4, 8) to the matching SDL_GPUSampleCount enum value.
SDL_GPUSampleCount EgGpusSdl_SampleCountToEnum(uint32_t sample_count);
