#pragma once

#include "stb_truetype.h"

#define FONT_FIRST_CHAR  32
#define FONT_CHAR_COUNT  96
#define FONT_BAKE_SIZE   32.0f
#define FONT_LINE_HEIGHT (FONT_BAKE_SIZE * 1.2f)

/* The atlas must be initialized with EgDraws_Font_GetBitmap before use. */
void font_get_quad(int codepoint, float *cursorX, float *cursorY, stbtt_aligned_quad *quad);
