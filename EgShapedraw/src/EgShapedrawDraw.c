#include "EgShapedraw.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_truetype.h"

#define EG_PI              3.14159265358979323846f
#define FONT_FIRST_CHAR    32
#define FONT_CHAR_COUNT    96
#define FONT_BAKE_SIZE     32.0f
#define FONT_LINE_HEIGHT   (FONT_BAKE_SIZE * 1.2f)

typedef struct {
	float x;
	float y;
	float c;
	float s;
} sXform_t;

typedef struct {
	EgShapedrawLayer *layer;
	float             pixelScale;
	sXform_t          xf;
	uint8_t           r;
	uint8_t           g;
	uint8_t           b;
	uint8_t           a;
} sBatch_t;

static stbtt_bakedchar sGlyphs[FONT_CHAR_COUNT];
static unsigned char   sBitmap[EG_SHAPEDRAW_ATLAS_WIDTH * EG_SHAPEDRAW_ATLAS_HEIGHT];
static int             sFontState; // 0 = not baked, 1 = ok, -1 = failed

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

const unsigned char *EgShapedrawFont_GetBitmap(void)
{
	if (sFontState == 0) {
		size_t         fontSize = 0;
		unsigned char *fontData = sLoadSystemFont(&fontSize);
		if (fontData == NULL) {
			fprintf(stderr, "EgShapedraw: failed to locate a default TrueType font\n");
			sFontState = -1;
		} else {
			memset(sBitmap, 0, sizeof(sBitmap));
			int rowUsed = stbtt_BakeFontBitmap(fontData, 0, FONT_BAKE_SIZE, sBitmap, EG_SHAPEDRAW_ATLAS_WIDTH, EG_SHAPEDRAW_ATLAS_HEIGHT, FONT_FIRST_CHAR, FONT_CHAR_COUNT, sGlyphs);
			free(fontData);
			sFontState = rowUsed > 0 ? 1 : -1;
		}
	}

	return sFontState == 1 ? sBitmap : NULL;
}

static void sAppendVertex(EgShapedrawLayer *l, float x, float y, sXform_t xf, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	if (l->count >= l->capacity) {
		int32_t            cap  = l->capacity > 0 ? l->capacity * 2 : 256;
		EgShapedrawVertex *data = realloc(l->data, (size_t)cap * sizeof(EgShapedrawVertex));
		if (data == NULL) {
			return;
		}
		l->data     = data;
		l->capacity = cap;
	}

	EgShapedrawVertex *dst = &l->data[l->count++];
	dst->position[0]       = xf.c * x - xf.s * y + xf.x;
	dst->position[1]       = xf.s * x + xf.c * y + xf.y;
	dst->uv[0]             = u;
	dst->uv[1]             = v;
	dst->rgba[0]           = r;
	dst->rgba[1]           = g;
	dst->rgba[2]           = b;
	dst->rgba[3]           = a;
}

static void sAddQuad(EgShapedrawLayer *l, float x0, float y0, float x1, float y1, sXform_t xf, float u0, float v0, float u1, float v1, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	sAppendVertex(l, x0, y0, xf, u0, v0, r, g, b, a);
	sAppendVertex(l, x1, y0, xf, u1, v0, r, g, b, a);
	sAppendVertex(l, x1, y1, xf, u1, v1, r, g, b, a);
	sAppendVertex(l, x0, y0, xf, u0, v0, r, g, b, a);
	sAppendVertex(l, x1, y1, xf, u1, v1, r, g, b, a);
	sAppendVertex(l, x0, y1, xf, u0, v1, r, g, b, a);
}

static void sAddTriangle(const sBatch_t *batch, float x0, float y0, float x1, float y1, float x2, float y2)
{
	const float whiteU = 0.5f / EG_SHAPEDRAW_ATLAS_WIDTH;
	const float whiteV = 0.5f / EG_SHAPEDRAW_ATLAS_HEIGHT;
	sAppendVertex(batch->layer, x0, y0, batch->xf, whiteU, whiteV, batch->r, batch->g, batch->b, batch->a);
	sAppendVertex(batch->layer, x1, y1, batch->xf, whiteU, whiteV, batch->r, batch->g, batch->b, batch->a);
	sAppendVertex(batch->layer, x2, y2, batch->xf, whiteU, whiteV, batch->r, batch->g, batch->b, batch->a);
}

static void sAddSolidQuad(const sBatch_t *batch, float x0, float y0, float x1, float y1)
{
	const float whiteU = 0.5f / EG_SHAPEDRAW_ATLAS_WIDTH;
	const float whiteV = 0.5f / EG_SHAPEDRAW_ATLAS_HEIGHT;
	sAddQuad(batch->layer, x0, y0, x1, y1, batch->xf, whiteU, whiteV, whiteU, whiteV, batch->r, batch->g, batch->b, batch->a);
}

// Fetches the z layer, stores the transform applied to appended vertices and decodes the color.
static int sBeginBatch(sBatch_t *batch, EgShapedrawList *list, int32_t z, float x, float y, float c, float s, uint32_t color)
{
	batch->layer = EgShapedrawList_GetLayer(list, z);
	if (batch->layer == NULL) {
		return 0;
	}

	batch->pixelScale = list->pixelScale > 0.0f ? list->pixelScale : 1.0f;
	batch->xf         = (sXform_t){x, y, c, s};
	batch->r          = (uint8_t)((color >> 16) & 0xFF);
	batch->g          = (uint8_t)((color >> 8) & 0xFF);
	batch->b          = (uint8_t)(color & 0xFF);
	batch->a          = (uint8_t)((color >> 24) & 0xFF);
	if (batch->a == 0) {
		batch->a = 255;
	}
	return 1;
}

void EgShapedrawList_AddTriangle(EgShapedrawList *list, int32_t z, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color)
{
	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, 0.0f, 0.0f, 1.0f, 0.0f, color)) {
		return;
	}

	sAddTriangle(&batch, x0, y0, x1, y1, x2, y2);
}

static void sAddLine(const sBatch_t *batch, float x1, float y1, float x2, float y2, float thickness)
{
	if (thickness <= 0.0f) {
		return;
	}

	float dx     = x2 - x1;
	float dy     = y2 - y1;
	float length = sqrtf(dx * dx + dy * dy);
	if (length <= 0.0f) {
		return;
	}

	float halfThickness = thickness * batch->pixelScale * 0.5f;
	float nx            = -dy / length * halfThickness;
	float ny            = dx / length * halfThickness;

	sAddTriangle(batch, x1 + nx, y1 + ny, x1 - nx, y1 - ny, x2 + nx, y2 + ny);
	sAddTriangle(batch, x1 - nx, y1 - ny, x2 - nx, y2 - ny, x2 + nx, y2 + ny);
}

static void sAddTextQuad(const sBatch_t *batch, const m3f32 *transform, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1)
{
	const float positions[4][3] = {
	{x0, y0, 1.0f},
	{x1, y0, 1.0f},
	{x1, y1, 1.0f},
	{x0, y1, 1.0f},
	};
	float transformed[4][3];
	for (int i = 0; i < 4; ++i) {
		m3f32_mulv(transform, positions[i], transformed[i]);
	}

	sXform_t identity = {0.0f, 0.0f, 1.0f, 0.0f};
	sAppendVertex(batch->layer, transformed[0][0], transformed[0][1], identity, u0, v0, batch->r, batch->g, batch->b, batch->a);
	sAppendVertex(batch->layer, transformed[1][0], transformed[1][1], identity, u1, v0, batch->r, batch->g, batch->b, batch->a);
	sAppendVertex(batch->layer, transformed[2][0], transformed[2][1], identity, u1, v1, batch->r, batch->g, batch->b, batch->a);
	sAppendVertex(batch->layer, transformed[0][0], transformed[0][1], identity, u0, v0, batch->r, batch->g, batch->b, batch->a);
	sAppendVertex(batch->layer, transformed[2][0], transformed[2][1], identity, u1, v1, batch->r, batch->g, batch->b, batch->a);
	sAppendVertex(batch->layer, transformed[3][0], transformed[3][1], identity, u0, v1, batch->r, batch->g, batch->b, batch->a);
}

void EgShapedrawList_AddText(EgShapedrawList *list, int32_t z, const m3f32 *transform, float fontSize, uint32_t color, const char *string)
{
	if (transform == NULL || string == NULL || EgShapedrawFont_GetBitmap() == NULL) {
		return;
	}

	float scale = fontSize / FONT_BAKE_SIZE;
	if (scale <= 0.0f) {
		return;
	}

	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, 0.0f, 0.0f, 1.0f, 0.0f, color)) {
		return;
	}

	float cursorX = 0.0f;
	float cursorY = 0.0f;
	float startX  = 0.0f;

	for (const char *p = string; *p != '\0'; ++p) {
		int codepoint = (unsigned char)*p;
		if (codepoint == '\n') {
			cursorX = startX;
			cursorY -= FONT_LINE_HEIGHT * scale;
			continue;
		}

		if (codepoint == '\t') {
			cursorX += 4.0f * FONT_LINE_HEIGHT * 0.5f * scale;
			continue;
		}

		if (codepoint < FONT_FIRST_CHAR || codepoint >= FONT_FIRST_CHAR + FONT_CHAR_COUNT) {
			codepoint = '?';
		}

		stbtt_aligned_quad q;
		stbtt_GetBakedQuad(sGlyphs, EG_SHAPEDRAW_ATLAS_WIDTH, EG_SHAPEDRAW_ATLAS_HEIGHT, codepoint - FONT_FIRST_CHAR, &cursorX, &cursorY, &q, 1);

		float dx0 = q.x0 - startX;
		float dy0 = q.y0 - cursorY;
		float dx1 = q.x1 - startX;
		float dy1 = q.y1 - cursorY;
		q.x0      = startX + scale * dx0;
		q.y0      = cursorY - scale * dy0;
		q.x1      = startX + scale * dx1;
		q.y1      = cursorY - scale * dy1;

		sAddTextQuad(&batch, transform, q.x0, q.y0, q.x1, q.y1, q.s0, q.t0, q.s1, q.t1);
	}
}

void EgShapedrawList_AddLine(EgShapedrawList *list, int32_t z, float x1, float y1, float x2, float y2, float thickness, uint32_t color)
{
	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, 0.0f, 0.0f, 1.0f, 0.0f, color)) {
		return;
	}

	sAddLine(&batch, x1, y1, x2, y2, thickness);
}

void EgShapedrawList_AddPoint(EgShapedrawList *list, int32_t z, float x, float y, float size, uint32_t color)
{
	if (size <= 0.0f) {
		return;
	}

	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, x, y, 1.0f, 0.0f, color)) {
		return;
	}

	float scaledSize = size * batch.pixelScale;
	sAddSolidQuad(&batch, -scaledSize, -scaledSize, scaledSize, scaledSize);
}

void EgShapedrawList_AddCircle(EgShapedrawList *list, int32_t z, float x, float y, float radius, uint32_t color)
{
	if (radius <= 0.0f) {
		return;
	}

	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, x, y, 1.0f, 0.0f, color)) {
		return;
	}

	int segments = 32;
	for (int i = 0; i < segments; ++i) {
		float angle0 = (float)i * (2.0f * EG_PI / (float)segments);
		float angle1 = (float)(i + 1) * (2.0f * EG_PI / (float)segments);
		sAddTriangle(&batch, 0.0f, 0.0f, cosf(angle0) * radius, sinf(angle0) * radius, cosf(angle1) * radius, sinf(angle1) * radius);
	}
}

void EgShapedrawList_AddCircleOutline(EgShapedrawList *list, int32_t z, float x, float y, float radius, float thickness, uint32_t color)
{
	if (radius <= 0.0f || thickness <= 0.0f) {
		return;
	}

	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, 0.0f, 0.0f, 1.0f, 0.0f, color)) {
		return;
	}

	int   segments        = 32;
	float scaledThickness = thickness * batch.pixelScale;
	float innerRadius     = radius - scaledThickness * 0.5f;
	float outerRadius     = radius + scaledThickness * 0.5f;
	if (innerRadius < 0.0f) {
		innerRadius = 0.0f;
	}

	for (int i = 0; i < segments; ++i) {
		float angle0 = (float)i * (2.0f * EG_PI / (float)segments);
		float angle1 = (float)(i + 1) * (2.0f * EG_PI / (float)segments);

		float x0Outer = x + cosf(angle0) * outerRadius;
		float y0Outer = y + sinf(angle0) * outerRadius;
		float x1Outer = x + cosf(angle1) * outerRadius;
		float y1Outer = y + sinf(angle1) * outerRadius;
		float x0Inner = x + cosf(angle0) * innerRadius;
		float y0Inner = y + sinf(angle0) * innerRadius;
		float x1Inner = x + cosf(angle1) * innerRadius;
		float y1Inner = y + sinf(angle1) * innerRadius;

		sAddTriangle(&batch, x0Outer, y0Outer, x1Outer, y1Outer, x1Inner, y1Inner);
		sAddTriangle(&batch, x0Outer, y0Outer, x1Inner, y1Inner, x0Inner, y0Inner);
	}
}

// Adds one half-ring cap centred on (cx, cy) sweeping from startAngle to startAngle + PI.
static void sAddCap(const sBatch_t *batch, float cx, float cy, float startAngle, float radius, float innerRadius, int segments)
{
	for (int i = 0; i < segments; ++i) {
		float angle0 = startAngle + (float)i / (float)segments * EG_PI;
		float angle1 = startAngle + (float)(i + 1) / (float)segments * EG_PI;

		float ox0 = cx + cosf(angle0) * radius;
		float oy0 = cy + sinf(angle0) * radius;
		float ox1 = cx + cosf(angle1) * radius;
		float oy1 = cy + sinf(angle1) * radius;
		float ix0 = cx + cosf(angle0) * innerRadius;
		float iy0 = cy + sinf(angle0) * innerRadius;
		float ix1 = cx + cosf(angle1) * innerRadius;
		float iy1 = cy + sinf(angle1) * innerRadius;

		sAddTriangle(batch, ox0, oy0, ox1, oy1, ix1, iy1);
		sAddTriangle(batch, ox0, oy0, ix1, iy1, ix0, iy0);
	}
}

void EgShapedrawList_AddCapsuleOutline(EgShapedrawList *list, int32_t z, float x1, float y1, float x2, float y2, float radius, float thickness, uint32_t color)
{
	if (radius <= 0.0f || thickness <= 0.0f) {
		return;
	}

	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, 0.0f, 0.0f, 1.0f, 0.0f, color)) {
		return;
	}

	float dx     = x2 - x1;
	float dy     = y2 - y1;
	float length = sqrtf(dx * dx + dy * dy);
	if (length <= 0.0f) {
		return;
	}

	float nx         = dx / length;
	float ny         = dy / length;
	float halfLength = length * 0.5f;
	float centerX    = (x1 + x2) * 0.5f;
	float centerY    = (y1 + y2) * 0.5f;

	sAddLine(&batch, x1, y1, x2, y2, thickness);

	int   segments    = 24;
	float innerRadius = radius - thickness * batch.pixelScale * 0.5f;
	sAddCap(&batch, centerX + nx * halfLength, centerY + ny * halfLength, EG_PI, radius, innerRadius, segments);
	sAddCap(&batch, centerX - nx * halfLength, centerY - ny * halfLength, 0.0f, radius, innerRadius, segments);
}

void EgShapedrawList_AddTransform(EgShapedrawList *list, int32_t z, float x, float y, float rotationCos, float rotationSin, float scale, uint32_t color)
{
	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, x, y, rotationCos, rotationSin, color)) {
		return;
	}

	sAddLine(&batch, 0.0f, 0.0f, scale, 0.0f, 0.05f);
	sAddLine(&batch, 0.0f, 0.0f, 0.0f, scale, 0.05f);
}

void EgShapedrawList_AddRectangle(EgShapedrawList *list, int32_t z, float x, float y, float rotationCos, float rotationSin, float width, float height, uint32_t color)
{
	if (width <= 0.0f || height <= 0.0f) {
		return;
	}

	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, x, y, rotationCos, rotationSin, color)) {
		return;
	}

	sAddSolidQuad(&batch, -width * 0.5f, -height * 0.5f, width * 0.5f, height * 0.5f);
}

void EgShapedrawList_AddRectangleOutline(EgShapedrawList *list, int32_t z, float x, float y, float rotationCos, float rotationSin, float width, float height, float thickness, uint32_t color)
{
	if (width <= 0.0f || height <= 0.0f || thickness <= 0.0f) {
		return;
	}

	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, x, y, rotationCos, rotationSin, color)) {
		return;
	}

	float hw = width * 0.5f;
	float hh = height * 0.5f;
	sAddLine(&batch, -hw, -hh, hw, -hh, thickness);
	sAddLine(&batch, hw, -hh, hw, hh, thickness);
	sAddLine(&batch, hw, hh, -hw, hh, thickness);
	sAddLine(&batch, -hw, hh, -hw, -hh, thickness);
}

void EgShapedrawList_AddBounds(EgShapedrawList *list, int32_t z, float minX, float minY, float maxX, float maxY, uint32_t color)
{
	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, 0.0f, 0.0f, 1.0f, 0.0f, color)) {
		return;
	}

	sAddLine(&batch, minX, minY, maxX, minY, 0.05f);
	sAddLine(&batch, maxX, minY, maxX, maxY, 0.05f);
	sAddLine(&batch, maxX, maxY, minX, maxY, 0.05f);
	sAddLine(&batch, minX, maxY, minX, minY, 0.05f);
}

void EgShapedrawList_AddPolygon(EgShapedrawList *list, int32_t z, const EgShapedrawVec2 *vertices, int vertexCount, float tx, float ty, float rotationCos, float rotationSin, uint32_t color)
{
	if (vertices == NULL || vertexCount < 3) {
		return;
	}

	sBatch_t batch;
	if (!sBeginBatch(&batch, list, z, tx, ty, rotationCos, rotationSin, color)) {
		return;
	}

	for (int i = 1; i + 1 < vertexCount; ++i) {
		sAddTriangle(&batch, vertices[0].x, vertices[0].y, vertices[i].x, vertices[i].y, vertices[i + 1].x, vertices[i + 1].y);
	}
}
