#include "misc.h"

void EcsMember_to_SDL_GPUVertexAttribute(SDL_GPUVertexAttribute *attribute, EcsMember const *member)
{
    int32_t count = member->count ? member->count : 1;
    attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
	attribute->offset = member->offset;
	if (member->type == ecs_id(ecs_f32_t)) {
        switch (count) {
            case 1: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT; break;
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; break;
            case 3: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4; break;
        }
	} else if (member->type == ecs_id(ecs_i32_t)) {
    switch (count) {
            case 1: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INT; break;
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INT2; break;
            case 3: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INT3; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_INT4; break;
        }
	} else if (member->type == ecs_id(ecs_u32_t)) {
    switch (count) {
            case 1: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UINT; break;
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UINT2; break;
            case 3: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UINT3; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UINT4; break;
        }
	} else if (member->type == ecs_id(ecs_i8_t)) {
		attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_BYTE2;
    switch (count) {
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_BYTE2; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_BYTE4; break;
        }
	} else if (member->type == ecs_id(ecs_u8_t)) {
    switch (count) {
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4; break;
        }
	} else if (member->type == ecs_id(ecs_i16_t)) {
    switch (count) {
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_SHORT2; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_SHORT4; break;
        }
	} else if (member->type == ecs_id(ecs_u16_t)) {
    switch (count) {
            case 2: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_USHORT2; break;
            case 4: attribute->format = SDL_GPU_VERTEXELEMENTFORMAT_USHORT4; break;
        }
	}
}


uint32_t EcsMember_to_SDL_GPUVertexAttribute_array(SDL_GPUVertexAttribute attribute[], ecs_world_t *world, ecs_entity_t type)
{
    uint32_t attribute_count = 0;
    ecs_iter_t it = ecs_children(world, type);
    while (ecs_children_next(&it)) {
        for (int i = 0; i < it.count; i++) {
            ecs_entity_t child = it.entities[i];
            const EcsMember *member = ecs_get(world, child, EcsMember);
            if (!member || attribute_count >= MAX_ATTRIBUTES) {
                continue;
            }
            EcsMember_to_SDL_GPUVertexAttribute(&attribute[attribute_count], member);
            attribute[attribute_count].location = attribute_count;
            attribute[attribute_count].buffer_slot = 0;
            attribute_count++;
        }
    }
    ecs_iter_fini(&it);
    return attribute_count;
}
