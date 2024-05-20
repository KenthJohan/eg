#pragma once
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <flecs.h>
#include <egquantities.h>


bool igCombo_flecs(ecs_world_t *world, ecs_entity_t parent, int *parent_val);


char const *flecs_get_type(ecs_primitive_kind_t kind);

size_t djb_hash(const char *cp);

void igPushStyleColor_U32_HSV_strhash(const char *cp);

void igPushStyleColor_U32_HSV_hash32(uint32_t value);

bool igSlider_flecs(const char *label, EgQuantitiesRangedGeneric *value);

void igInput_flecs(const char *label, eg_generic_number_t *val, ecs_primitive_kind_t kind);

void igText_flecs(ecs_world_t * world, ecs_entity_t type, eg_generic_number_t *value, ecs_primitive_kind_t kind);