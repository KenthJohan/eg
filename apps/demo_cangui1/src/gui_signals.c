#include "gui_signals.h"

#include <egquantities.h>
#include <egcan.h>

#include <egcolors/eg_color.h>

#define COLOR_RGBA(r, g, b, a) ((r) << 0 | (g) << 8 | (b) << 16 | (a) << 24)

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "GuiCan.h"

typedef struct {
	char const *name;
	EgCanBusDescription *desc;
	EgCanBus *bus;
	EgCanSignal *signal;
	EgQuantitiesRangedGeneric *value;
	EgQuantitiesIsq *q;
	ecs_entity_t e;
} gui_can_table_t;

static void igSlider_flecs(const char *label, eg_can_book_t *book, EgCanSignal *signal, EgQuantitiesRangedGeneric *value)
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
		EgCan_book_prepare_send(book, signal, value);
	}
}

static void igInput_flecs(const char *label, eg_generic_number_t *val, ecs_primitive_kind_t kind)
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

static char const *get_type(ecs_primitive_kind_t kind)
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

static size_t djb_hash(const char *cp)
{
	size_t hash = 5381;
	while (*cp)
		hash = 33 * hash ^ (unsigned char)*cp++;
	return hash;
}

static void igPushStyleColor_U32_HSV_strhash(const char *cp)
{
	size_t h = djb_hash(cp);
	uint8_t r;
	uint8_t g;
	uint8_t b;
	eg_color_hsv_to_rgb(h, h >> 16, 255, &r, &g, &b);
	igPushStyleColor_U32(ImGuiCol_Text, COLOR_RGBA(r, g, b, 255));
}

static void igPushStyleColor_U32_HSV_hash32(uint32_t value)
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

static bool igCombo_flecs(ecs_world_t *world, ecs_entity_t parent, int *parent_val)
{
	bool selected = false;
	const EcsEnum *enum_type = ecs_get(world, parent, EcsEnum);
	ecs_enum_constant_t *c0 = ecs_map_get_deref(&enum_type->constants, ecs_enum_constant_t, (ecs_map_key_t)*parent_val);
	igPushItemWidth(-1);
	igPushID_Ptr((void *)(intptr_t)parent);
	if (igBeginCombo("", c0 ? c0->name : NULL, 0)) {
		ecs_map_iter_t it = ecs_map_iter(&enum_type->constants);
		while (ecs_map_next(&it)) {
			ecs_enum_constant_t *c = ecs_map_ptr(&it);
			selected = c->value == c0->value;
			selected = igSelectable_Bool(c->name, selected, 0, (ImVec2){0, 0});
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

void gui_signals_progress(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);

	static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	if (igBeginTable("Signals table", 15, flags2, (ImVec2){0, 0}, 0)) {
		gui_can_table_t gui[128] = {0};
		int n = 0;
		ecs_iter_t it = ecs_query_iter(world, q);
		while (ecs_query_next(&it)) {
			EgCanBus *bus = ecs_field(&it, EgCanBus, 1);                                     // shared
			EgCanBusDescription *desc = ecs_field(&it, EgCanBusDescription, 2);              // shared
			EgCanSignal *signal = ecs_field(&it, EgCanSignal, 3);                            // self
			EgQuantitiesIsq *quant = ecs_field(&it, EgQuantitiesIsq, 4);                     // self, optional
			EgQuantitiesRangedGeneric *value = ecs_field(&it, EgQuantitiesRangedGeneric, 5); // self

			for (int i = 0; i < it.count; ++i, ++signal, ++quant, ++value) {
				ecs_entity_t e = it.entities[i];
				int list_index = signal->gui_index;
				if (list_index >= 128) {
					continue;
				}
				char const *name = ecs_get_path_w_sep(world, signal->gui_scope_name_parent, e, ".", NULL);
				// printf("e: %s, from : %s\n", name, name1 ? name1 : "?");
				gui[list_index].e = e;
				gui[list_index].name = name;
				gui[list_index].signal = signal;
				gui[list_index].value = value;
				gui[list_index].q = quant;
				gui[list_index].desc = desc;
				gui[list_index].bus = bus;
				n = ECS_MAX(list_index + 1, n);
			}
		}

		igTableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("bus", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("sock", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("id10", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("id16", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("idn", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("o", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("kind", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("min", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("max", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("tx", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("rx", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("q", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("u", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("plot", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableHeadersRow();

		/*
		ecs_os_mutex_lock(stuff->lock);
		memcpy(book->tx, book->rx, sizeof(eg_can_book8_t) * book->cap);
		ecs_os_mutex_unlock(stuff->lock);
		*/

		for (int i = 0; i < n; ++i) {
			// ecs_entity_t e = gui[i].e;
			char const *name = gui[i].name;
			EgQuantitiesIsq *quant = gui[i].q;
			EgCanSignal *signal = gui[i].signal;
			EgQuantitiesRangedGeneric *value = gui[i].value;
			EgCanBus *bus = gui[i].bus;
			EgCanBusDescription *desc = gui[i].desc;
			ecs_entity_t e = gui[i].e;

			if (name == NULL) {
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				continue;
			}

			eg_can_book_t *book = bus->ptr;
			igPushID_Ptr(signal);
			igTableNextColumn();
			igText("%s", name);
			igTableNextColumn();
			igText(desc->interface);
			igTableNextColumn();
			igText("%i", bus->socket);

			igPushStyleColor_U32_HSV_hash32(signal->canid);
			igTableNextColumn();
			igText("%i", signal->canid);
			igTableNextColumn();
			igText("%03X", signal->canid);
			igPopStyleColor(1);

			igTableNextColumn();
			igText("%i", signal->idn);
			igTableNextColumn();
			igText("%i", signal->byte_offset);
			igTableNextColumn();
			igPushStyleColor_U32_HSV_strhash(get_type(value->kind));
			igText("%s", get_type(value->kind));
			igPopStyleColor(1);

			igTableNextColumn();
			igInput_flecs("#1", &value->min, value->kind);

			igTableNextColumn();
			igInput_flecs("#2", &value->max, value->kind);

			igTableNextColumn();
			if (signal->component_rep && ecs_has(world, signal->component_rep, EcsEnum)) {
				if (value == NULL) {
					// printf("e: %s\n", ecs_get_name(world, value));
					return;
				}
				int selected = (int)value->tx.val_u64;
				bool changed = igCombo_flecs(world, signal->component_rep, &selected);
				if (changed) {
					value->tx.val_u64 = (int32_t)selected;
				}
			} else {
				igSlider_flecs("##s1", book, signal, value);
			}

			/*
			if (igSliderScalar("##s1", ImGuiDataType_Float, &value->tx.val_f32, &value->min.val_f32, &value->max.val_f32, "%f", 0)) {
			    EgCan_book_prepare_send(book, signal, value);
			};
			*/

			igTableNextColumn();
			switch (value->kind) {
			case EcsU8:
				igText("%i", value->rx.val_u8);
				break;
			case EcsU16:
				igText("%i", value->rx.val_u16);
				break;
			case EcsU32:
				igText("%i", value->rx.val_u32);
				break;
			case EcsF32:
				igText("%f", value->rx.val_f32);
				break;

			default:
				break;
			}
			/*
			if (signal->min != signal->max) {
			    igBeginDisabled(true);
			    igPushItemWidth(-1);
			    igSliderScalar("##s2", ImGuiDataType_S32, &signal->rx, &signal->min, &signal->max, "%d", 0);
			    igPopItemWidth();
			    igEndDisabled();
			} else {
			    igText("");
			}
			*/

			/*
			igTableNextColumn();
			igBeginDisabled(true);
			igText("%i", signal->rx);
			igEndDisabled();
			*/

			igTableNextColumn();
			igText(quant ? quant->symbol : "");
			igTableNextColumn();
			igText("");

			igTableNextColumn();
			{
				bool checked = ecs_has(world, gui[i].e, GuiCanPlot);
				bool pressed = igCheckbox("", &checked);
				if (pressed) {
					printf("checked:%i:%s\n", checked, name);
					if (checked) {
						ecs_add(world, gui[i].e, GuiCanPlot);
					} else {
						ecs_remove(world, gui[i].e, GuiCanPlot);
					}
				}
			}

			igPopID();
		}

		igEndTable();
	}
}

ecs_query_t *gui_signals_query(ecs_world_t *world)
{
	// clang-format off
	ecs_query_t * q = ecs_query(world, {
		.filter.terms = {
			{.id = ecs_id(EgCanBus), .src.flags = EcsUp, .src.trav = EcsChildOf},
			{.id = ecs_id(EgCanBusDescription), .src.flags = EcsUp, .src.trav = EcsChildOf},
			{.id = ecs_id(EgCanSignal), .src.flags = EcsSelf}, // EcsSelf is temporary fix to only query from "app.signals".
			{.id = ecs_id(EgQuantitiesIsq), .oper = EcsOptional},
			{.id = ecs_id(EgQuantitiesRangedGeneric), .src.flags = EcsSelf}, // EcsSelf is temporary fix to only query from "app.signals".
			//{.id = ecs_id(EgCanSignal)},
			// TODO: Only query entities from "app.signals"
			//{.id = EcsModule, .src.id = e_app_signals},
		}
		}
	);
	// clang-format on
	return q;
}