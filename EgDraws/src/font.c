#include "font.h"
#include "EgDraws.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static stbtt_bakedchar sGlyphs[FONT_CHAR_COUNT];
static unsigned char sBitmap[EG_DRAWS_ATLAS_WIDTH * EG_DRAWS_ATLAS_HEIGHT];
static int sFontState; // 0 = not baked, 1 = ok, -1 = failed

static unsigned char *sReadBinaryFile(const char *path, size_t *outSize)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		return NULL;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return NULL;
	}

	long size = ftell(file);
	if (size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
		fclose(file);
		return NULL;
	}

	unsigned char *bytes = malloc((size_t)size);
	if (bytes == NULL) {
		fclose(file);
		return NULL;
	}

	if (fread(bytes, 1, (size_t)size, file) != (size_t)size) {
		fclose(file);
		free(bytes);
		return NULL;
	}

	fclose(file);
	*outSize = (size_t)size;
	return bytes;
}

static unsigned char *sLoadSystemFont(size_t *outSize)
{
	const char *candidates[] = {
	"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
	"/usr/share/fonts/TTF/DejaVuSans.ttf",
	"/usr/share/fonts/dejavu/DejaVuSans.ttf",
	"C:/Windows/Fonts/arial.ttf",
	"/System/Library/Fonts/Supplemental/Arial.ttf",
	};

	for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
		unsigned char *fontData = sReadBinaryFile(candidates[i], outSize);
		if (fontData != NULL) {
			return fontData;
		}
	}

	return NULL;
}

const unsigned char *EgDraws_Font_GetBitmap(void)
{
	if (sFontState == 0) {
		size_t fontSize = 0;
		unsigned char *fontData = sLoadSystemFont(&fontSize);
		if (fontData == NULL) {
			fprintf(stderr, "EgDraws: failed to locate a default TrueType font\n");
			sFontState = -1;
		} else {
			memset(sBitmap, 0, sizeof(sBitmap));
			int rowUsed = stbtt_BakeFontBitmap(fontData, 0, FONT_BAKE_SIZE, sBitmap, EG_DRAWS_ATLAS_WIDTH, EG_DRAWS_ATLAS_HEIGHT, FONT_FIRST_CHAR, FONT_CHAR_COUNT, sGlyphs);
			free(fontData);
			sFontState = rowUsed > 0 ? 1 : -1;
			if (sFontState < 0) {
				fprintf(stderr, "EgDraws: failed to bake the default font atlas\n");
			}
		}
	}

	return sFontState == 1 ? sBitmap : NULL;
}

void font_get_quad(int codepoint, float *cursorX, float *cursorY, stbtt_aligned_quad *quad)
{
	stbtt_GetBakedQuad(sGlyphs, EG_DRAWS_ATLAS_WIDTH, EG_DRAWS_ATLAS_HEIGHT,
		codepoint - FONT_FIRST_CHAR, cursorX, cursorY, quad, 1);
}
