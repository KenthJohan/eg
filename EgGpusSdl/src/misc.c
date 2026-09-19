#include "misc.h"

void EcsMember_to_SDL_GPUVertexAttribute(SDL_GPUVertexAttribute *attribute, EcsMember const *member)
{
    attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
	attribute->offset = member->offset;
	if (member->type == ecs_id(ecs_f32_t)) {
        switch (member->count) {
            case 1: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT; break;
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; break;
            case 3: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4; break;
        }
	} else if (member->type == ecs_id(ecs_i32_t)) {
        switch (member->count) {
            case 1: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INT; break;
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INT2; break;
            case 3: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INT3; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INT4; break;
        }
	} else if (member->type == ecs_id(ecs_u32_t)) {
        switch (member->count) {
            case 1: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UINT; break;
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UINT2; break;
            case 3: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UINT3; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UINT4; break;
        }
	} else if (member->type == ecs_id(ecs_i8_t)) {
		attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_BYTE2;
        switch (member->count) {
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_BYTE2; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_BYTE4; break;
        }
	} else if (member->type == ecs_id(ecs_u8_t)) {
        switch (member->count) {
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4; break;
        }
	} else if (member->type == ecs_id(ecs_i16_t)) {
        switch (member->count) {
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_SHORT2; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_SHORT4; break;
        }
	} else if (member->type == ecs_id(ecs_u16_t)) {
        switch (member->count) {
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_USHORT2; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_USHORT4; break;
        }
	}
}


void EcsMember_to_SDL_GPUVertexAttribute_array(SDL_GPUVertexAttribute attribute[], ecs_world_t *world, ecs_entity_t type)
{
    ecs_type_t const * type_ptr = ecs_get_type(world, type);
    return;
}
