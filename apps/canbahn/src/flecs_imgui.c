#include "flecs_imgui.h"

#include <egcolors/eg_color.h>
#include <math.h>
#include <float.h>


#define COLOR_RGBA(r, g, b, a) ((r) << 0 | (g) << 8 | (b) << 16 | (a) << 24)

bool igCombo_flecs(ecs_world_t *world, ecs_entity_t parent, int *parent_val)
{
	bool selected = false;
	const EcsEnum *enum_type = ecs_get(world, parent, EcsEnum);
	ecs_enum_constant_t *c0 = ecs_map_get_deref(&enum_type->constants, ecs_enum_constant_t, (ecs_map_key_t)*parent_val);
	igPushItemWidth(-1);
	igPushID_Ptr((void *)(intptr_t)parent);
	char buf[128] = {0};
	if (c0) {
		snprintf(buf, sizeof(buf), "%s (%i)", c0->name, c0->value);
	}
	if (igBeginCombo("", buf, 0)) {
		ecs_map_iter_t it = ecs_map_iter(&enum_type->constants);
		while (ecs_map_next(&it)) {
			ecs_enum_constant_t *c = ecs_map_ptr(&it);
			selected = c->value == c0->value;
			snprintf(buf, sizeof(buf), "%s (%i)", c->name, c->value);
			selected = igSelectable_Bool(buf, selected, 0, (ImVec2){0, 0});
			if(selected) {
				*parent_val = c->value;
				break;
			}
		}
		igEndCombo();
	}
	igPopID();
	igPopItemWidth();
	return selected;
}


char const *flecs_get_type(ecs_primitive_kind_t kind)
{
	switch (kind) {
	case EcsBool:
		return "Bool";
	case EcsChar:
		return "Char";
	case EcsByte:
		return "Byte";
	case EcsU8:
		return "U8";
	case EcsU16:
		return "U16";
	case EcsU32:
		return "U32";
	case EcsU64:
		return "U64";
	case EcsI8:
		return "I8";
	case EcsI16:
		return "I16";
	case EcsI32:
		return "I32";
	case EcsI64:
		return "I64";
	case EcsF32:
		return "F32";
	case EcsF64:
		return "F64";
	case EcsUPtr:
		return "UPtr";
	case EcsIPtr:
		return "IPtr";
	case EcsString:
		return "String";
	case EcsEntity:
		return "Entity";
	case EcsId:
		return "Id";
	}
	return "";
}



size_t djb_hash(const char *cp)
{
	size_t hash = 5381;
	while (*cp)
		hash = 33 * hash ^ (unsigned char)*cp++;
	return hash;
}

void igPushStyleColor_U32_HSV_strhash(const char *cp)
{
	size_t h = djb_hash(cp);
	uint8_t r;
	uint8_t g;
	uint8_t b;
	eg_color_hsv_to_rgb(h, h >> 16, 255, &r, &g, &b);
	igPushStyleColor_U32(ImGuiCol_Text, COLOR_RGBA(r, g, b, 255));
}

void igPushStyleColor_U32_HSV_hash32(uint32_t value)
{
	size_t hash;
	hash = 5381;
	hash = 14 * hash ^ value;
	hash = 546567 * hash ^ value;
	uint8_t h = hash;
	hash = 5381;
	hash = 33 * hash ^ value;
	hash = 33333 * hash ^ value;
	uint8_t s = hash;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	eg_color_hsv_to_rgb(h, s, 255, &r, &g, &b);
	igPushStyleColor_U32(ImGuiCol_Text, COLOR_RGBA(r, g, b, 255));
}







bool igSlider_flecs(const char *label, EgQuantitiesRangedGeneric *value)
{
	bool modified = false;
	switch (value->kind) {
	case EcsF32:
		if (isfinite(value->tx.val_f32) && isfinite(value->min.val_f32) && isfinite(value->max.val_f32)) {
			igPushItemWidth(-1);
			modified = igSliderScalar(label, ImGuiDataType_Float, &value->tx.val_f32, &value->min.val_f32, &value->max.val_f32, "%f", 0);
			igPopItemWidth();
		}
		break;
	case EcsI8:
		igPushItemWidth(-1);
		int32_t min = value->min_i64;
		int32_t max = value->max_i64;
		int32_t v = value->tx.val_i8;
		modified = igSliderScalar(label, ImGuiDataType_S32, &v, &min, &max, "%i", 0);
		value->tx.val_i8 = v;
		igPopItemWidth();
		break;
	case EcsU8:
		igPushItemWidth(-1);
		modified = igSliderScalar(label, ImGuiDataType_U8, &value->tx.val_u8, &value->min.val_u8, &value->max.val_u8, "%u", 0);
		igPopItemWidth();
		break;
	case EcsU16:
		igPushItemWidth(-1);
		modified = igSliderScalar(label, ImGuiDataType_U16, &value->tx.val_u16, &value->min.val_u16, &value->max.val_u16, "%u", 0);
		igPopItemWidth();
		break;
	case EcsU32:
		igPushItemWidth(-1);
		modified = igSliderScalar(label, ImGuiDataType_U32, &value->tx.val_u32, &value->min.val_u32, &value->max.val_u32, "%u", 0);
		igPopItemWidth();
		break;

	default:
		break;
	}
	if (modified) {
		//EgCan_book_prepare_send(book, signal, value);
	}
	return modified;
}

void igInput_flecs(const char *label, eg_generic_number_t *val, ecs_primitive_kind_t kind)
{
	switch (kind) {
	case EcsF64:
		igPushItemWidth(-1);
		igInputDouble("#1", &val->val_f64, 0, 0, "%f", 0);
		igPopItemWidth();
		break;
	case EcsF32:
		igPushItemWidth(-1);
		igInputFloat("#1", &val->val_f32, 0, 0, "%f", 0);
		igPopItemWidth();
		break;
	case EcsU8:
		igPushItemWidth(-1);
		igInputScalar("#1", ImGuiDataType_U8, &val->val_u8, 0, 0, "%u", 0);
		igPopItemWidth();
		break;
	case EcsU16:
		igPushItemWidth(-1);
		igInputScalar("#1", ImGuiDataType_U16, &val->val_u16, 0, 0, "%u", 0);
		igPopItemWidth();
		break;
	case EcsU32:
		igPushItemWidth(-1);
		igInputScalar("#1", ImGuiDataType_U32, &val->val_u32, 0, 0, "%u", 0);
		igPopItemWidth();
		break;
	default:
		break;
	}
}


void igText_flecs_enum(ecs_world_t * world, ecs_entity_t type, eg_generic_number_t *value, ecs_primitive_kind_t kind)
{
	if (type && ecs_has(world, type, EcsEnum)) {
		int val = value->val_u64;
		const EcsEnum *enum_type = ecs_get(world, type, EcsEnum);
		ecs_enum_constant_t *c0 = ecs_map_get_deref(&enum_type->constants, ecs_enum_constant_t, (ecs_map_key_t)val);
		if (c0) {
			igText("%s (%i)", c0->name, c0->value);
		} else {
			igText("? (%i)", val);
		}
		return;
	}


	switch (kind) {
	case EcsI8:
		igText("%i", value->val_i8);
		break;
	case EcsU8:
		igText("%i", value->val_u8);
		break;
	case EcsU16:
		igText("%i", value->val_u16);
		break;
	case EcsU32:
		igText("%i", value->val_u32);
		break;
	case EcsF32:
		igText("%f", value->val_f32);
		break;
	default:
		break;
	}
}



void igText_flecs(eg_generic_number_t *value, ecs_primitive_kind_t kind)
{
	switch (kind) {
	case EcsU8:
		igText("%u", value->val_u8);
		break;
	case EcsU16:
		igText("%u", value->val_u16);
		break;
	case EcsU32:
		igText("%u", value->val_u32);
		break;
	case EcsF32:
		igText("%f", value->val_f32);
		break;
	default:
		break;
	}
}




void igCheckbox_flecs(ecs_world_t * world, ecs_entity_t e, ecs_id_t tag)
{
	bool checked = ecs_has_id(world, e, tag);
	bool pressed = igCheckbox("", &checked);
	if (pressed) {
		//printf("checked:%i:%s\n", checked, name);
		if (checked) {
			ecs_add_id(world, e, tag);
		} else {
			ecs_remove_id(world, e, tag);
		}
	}
}