#include "ig.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

static void color_hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t *out_r, uint8_t *out_g, uint8_t *out_b)
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t region;
	uint8_t remainder;
	uint8_t p;
	uint8_t q;
	uint8_t t;

	if (s == 0) {
		out_r[0] = v;
		out_g[0] = v;
		out_b[0] = v;
		return;
	}

	region = h / 43;
	remainder = (h - (region * 43)) * 6;

	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * remainder) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

	switch (region) {
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	default:
		r = v;
		g = p;
		b = q;
		break;
	}

	out_r[0] = r;
	out_g[0] = g;
	out_b[0] = b;
}

static void color_rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, uint8_t *out_h, uint8_t *out_s, uint8_t *out_v)
{
	uint8_t rgbMin;
	uint8_t rgbMax;

	rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
	rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);

	out_v[0] = rgbMax;
	if (out_v[0] == 0) {
		out_h[0] = 0;
		out_s[0] = 0;
		return;
	}

	out_s[0] = 255 * (long)(rgbMax - rgbMin) / out_v[0];
	if (out_s[0] == 0) {
		out_h[0] = 0;
		return;
	}

	if (rgbMax == r) {
		out_h[0] = 0 + 43 * (g - b) / (rgbMax - rgbMin);
	} else if (rgbMax == g) {
		out_h[0] = 85 + 43 * (b - r) / (rgbMax - rgbMin);
	} else {
		out_h[0] = 171 + 43 * (r - g) / (rgbMax - rgbMin);
	}
}

size_t djb_hash(const char *cp)
{
	size_t hash = 5381;
	while (*cp) {
		hash = 33 * (hash ^ (unsigned char)*cp) ^ *cp;
		cp++;
	}
	return hash;
}

void igPushStyleColor_U32_HSV_strhash(const char *cp)
{
	size_t h = djb_hash(cp);
	uint8_t r;
	uint8_t g;
	uint8_t b;
	color_hsv_to_rgb(h, h >> 16, 255, &r, &g, &b);
	igPushStyleColor_U32(ImGuiCol_Text, IM_COL32(r, g, b, 255));
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
	color_hsv_to_rgb(h, s, 255, &r, &g, &b);
	igPushStyleColor_U32(ImGuiCol_Text, IM_COL32(r, g, b, 255));
}

uint32_t hash32_to_color32(uint32_t value, uint8_t v)
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
	color_hsv_to_rgb(h, s, v, &r, &g, &b);
	return IM_COL32(r, g, b, 255);
}






void generic_gui(generic_gui_t *item)
{
	igPushID_Int(item->id);
	switch (item->kind) {
	case GENERIC_GUI_KIND_TEXT_INT:
		igText("%i", item->text_int.value);
		break;

	case GENERIC_GUI_KIND_TEXT_SELECTABLE_INT: {
		char buf[128];
		snprintf(buf, 128, "%i", item->text_int.value);
		ImGuiSelectableFlags flags = ImGuiSelectableFlags_SpanAllColumns & 0;
		*item->selectable_int.selected = igSelectable_Bool(buf, *item->selectable_int.selected, flags, (ImVec2){0, 0});
	} break;

	case GENERIC_GUI_KIND_INPUT_TEXT:
		igPushItemWidth(-1);
		igInputText(item->label, item->input.data, item->input.data_size, 0, 0, 0);
		igPopItemWidth();
		break;

	case GENERIC_GUI_KIND_INPUT_INT:
		igPushItemWidth(-1);
		igInputInt(item->label, item->input.data, 0, 0, 0);
		igPopItemWidth();
		break;

	case GENERIC_GUI_KIND_INPUT_FLOAT:
		igPushItemWidth(-1);
		igInputFloat(item->label, item->input.data, 0, 0, "%f", 0);
		igPopItemWidth();
		break;

	case GENERIC_GUI_KIND_INPUT_DOUBLE:
		igPushItemWidth(-1);
		igInputDouble(item->label, item->input.data, 0, 0, "%f", 0);
		igPopItemWidth();
		break;

	default:
		break;
	}
	igPopID();
}


void ig_debug_draw()
{
	ImVec2 my_pos;
	igGetCursorScreenPos(&my_pos);
	if (igIsKeyDown_Nil(ImGuiKey_J)) {
		igDebugDrawItemRect(4278190335U); // Helper to draw a rectangle between GetItemRectMin() and GetItemRectMax()
		ImDrawList_AddCircleFilled(igGetForegroundDrawList_WindowPtr(NULL), my_pos, 3, IM_COL32(255, 0, 0, 255), 0);
	}
}
