#pragma once
#include <stdint.h>
#include <stdbool.h>

#include <stddef.h>

void igPushStyleColor_U32_HSV_strhash(const char *cp);

void igPushStyleColor_U32_HSV_hash32(uint32_t value);

// Helpers macros to generate 32-bits encoded colors
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IM_COL32_R_SHIFT 16
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 0
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#else
#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#endif
#define IM_COL32(R, G, B, A) (((ImU32)(A) << IM_COL32_A_SHIFT) | ((ImU32)(B) << IM_COL32_B_SHIFT) | ((ImU32)(G) << IM_COL32_G_SHIFT) | ((ImU32)(R) << IM_COL32_R_SHIFT))
#define IM_COL32_WHITE IM_COL32(255, 255, 255, 255) // Opaque white = 0xFFFFFFFF
#define IM_COL32_BLACK IM_COL32(0, 0, 0, 255)       // Opaque black
#define IM_COL32_BLACK_TRANS IM_COL32(0, 0, 0, 0)   // Transparent black = 0x00000000

uint32_t hash32_to_color32(uint32_t value, uint8_t v);


typedef enum {
	GENERIC_GUI_KIND_TEXT_INT,
	GENERIC_GUI_KIND_TEXT_SELECTABLE,
	GENERIC_GUI_KIND_TEXT_SELECTABLE_INT,
	GENERIC_GUI_KIND_INPUT_TEXT,
	GENERIC_GUI_KIND_INPUT_INT,
	GENERIC_GUI_KIND_INPUT_FLOAT,
	GENERIC_GUI_KIND_INPUT_DOUBLE,
} generic_gui_kind_t;

typedef struct {
	int id;
	generic_gui_kind_t kind;
	char const *label;
	union {
		struct {
			void *data;
			size_t data_size;
		} input;

		struct {
			int value;
		} text_int;

		struct {
			int value;
			bool *selected;
			float height;
		} selectable_int;
	};
} generic_gui_t;

void generic_gui(generic_gui_t *item);

void ig_debug_draw();