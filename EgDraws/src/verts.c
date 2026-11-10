#include "verts.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_truetype.h"

#define EG_PI            3.14159265358979323846f
#define FONT_FIRST_CHAR  32
#define FONT_CHAR_COUNT  96
#define FONT_BAKE_SIZE   32.0f
#define FONT_LINE_HEIGHT (FONT_BAKE_SIZE * 1.2f)

static stbtt_bakedchar sGlyphs[FONT_CHAR_COUNT];
static unsigned char   sBitmap[EG_DRAWS_ATLAS_WIDTH * EG_DRAWS_ATLAS_HEIGHT];
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

const unsigned char *EgDraws_Font_GetBitmap(void)
{
	if (sFontState == 0) {
		size_t         fontSize = 0;
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

static void sAppendVertex(ecs_vec_t *l, float x, float y, const m3f32 *transform, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	EgDraws_Vertex *dst = ecs_vec_append_t(NULL, l, EgDraws_Vertex);
	if (dst == NULL) {
		ecs_err("EgDraws: failed to append vertex");
		return;
	}

	float              position[3] = {x, y, 1.0f};
	float              transformed[3];
	m3f32_mulv(transform, position, transformed);
	dst->position[0] = transformed[0];
	dst->position[1] = transformed[1];
	dst->uv[0]       = u;
	dst->uv[1]       = v;
	dst->rgba[0]     = r;
	dst->rgba[1]     = g;
	dst->rgba[2]     = b;
	dst->rgba[3]     = a;
}

static void sAddQuad(ecs_vec_t *l, float x0, float y0, float x1, float y1, const m3f32 *transform, float u0, float v0, float u1, float v1, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	sAppendVertex(l, x0, y0, transform, u0, v0, r, g, b, a);
	sAppendVertex(l, x1, y0, transform, u1, v0, r, g, b, a);
	sAppendVertex(l, x1, y1, transform, u1, v1, r, g, b, a);
	sAppendVertex(l, x0, y0, transform, u0, v0, r, g, b, a);
	sAppendVertex(l, x1, y1, transform, u1, v1, r, g, b, a);
	sAppendVertex(l, x0, y1, transform, u0, v1, r, g, b, a);
}

static int sValid(ecs_vec_t *v, const verts_config_t *config)
{
	if (v == NULL || config == NULL) {
		ecs_err("EgDraws: vertex vector and configuration must not be null");
		return 0;
	}
	return 1;
}

static float sPixelScale(const verts_config_t *config)
{
	return config->pixscale > 0.0f ? config->pixscale : 1.0f;
}

static void sAddTriangle(ecs_vec_t *v, const verts_config_t *config, float x0, float y0, float x1, float y1, float x2, float y2)
{
	const float whiteU = 0.5f / EG_DRAWS_ATLAS_WIDTH;
	const float whiteV = 0.5f / EG_DRAWS_ATLAS_HEIGHT;
	sAppendVertex(v, x0, y0, &config->transform, whiteU, whiteV, config->r, config->g, config->b, config->a);
	sAppendVertex(v, x1, y1, &config->transform, whiteU, whiteV, config->r, config->g, config->b, config->a);
	sAppendVertex(v, x2, y2, &config->transform, whiteU, whiteV, config->r, config->g, config->b, config->a);
}

static void sAddSolidQuad(ecs_vec_t *v, const verts_config_t *config, float x0, float y0, float x1, float y1)
{
	const float whiteU = 0.5f / EG_DRAWS_ATLAS_WIDTH;
	const float whiteV = 0.5f / EG_DRAWS_ATLAS_HEIGHT;
	sAddQuad(v, x0, y0, x1, y1, &config->transform, whiteU, whiteV, whiteU, whiteV, config->r, config->g, config->b, config->a);
}

void verts_add_triangle(ecs_vec_t *v, const verts_config_t *config, float x0, float y0, float x1, float y1, float x2, float y2)
{
	if (!sValid(v, config)) {
		return;
	}

	sAddTriangle(v, config, x0, y0, x1, y1, x2, y2);
}

static void sAddLine(ecs_vec_t *v, const verts_config_t *config, float x1, float y1, float x2, float y2, float thickness)
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

	float halfThickness = thickness * sPixelScale(config) * 0.5f;
	float nx            = -dy / length * halfThickness;
	float ny            = dx / length * halfThickness;

	sAddTriangle(v, config, x1 + nx, y1 + ny, x1 - nx, y1 - ny, x2 + nx, y2 + ny);
	sAddTriangle(v, config, x1 - nx, y1 - ny, x2 - nx, y2 - ny, x2 + nx, y2 + ny);
}

static void sAddTextQuad(ecs_vec_t *v, const verts_config_t *config, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1)
{
	const float positions[4][3] = {
	{x0, y0, 1.0f},
	{x1, y0, 1.0f},
	{x1, y1, 1.0f},
	{x0, y1, 1.0f},
	};
	float transformed[4][3];
	for (int i = 0; i < 4; ++i) {
		m3f32_mulv(&config->transform, positions[i], transformed[i]);
	}

	m3f32 identity = M3_IDENTITY;
	sAppendVertex(v, transformed[0][0], transformed[0][1], &identity, u0, v0, config->r, config->g, config->b, config->a);
	sAppendVertex(v, transformed[1][0], transformed[1][1], &identity, u1, v0, config->r, config->g, config->b, config->a);
	sAppendVertex(v, transformed[2][0], transformed[2][1], &identity, u1, v1, config->r, config->g, config->b, config->a);
	sAppendVertex(v, transformed[0][0], transformed[0][1], &identity, u0, v0, config->r, config->g, config->b, config->a);
	sAppendVertex(v, transformed[2][0], transformed[2][1], &identity, u1, v1, config->r, config->g, config->b, config->a);
	sAppendVertex(v, transformed[3][0], transformed[3][1], &identity, u0, v1, config->r, config->g, config->b, config->a);
}

void verts_add_text(ecs_vec_t *v, const verts_config_t *config, float fontSize, const char *string)
{
	if (!sValid(v, config)) {
		return;
	}
	if (string == NULL) {
		ecs_err("EgDraws: text must not be null");
		return;
	}
	if (EgDraws_Font_GetBitmap() == NULL) {
		return;
	}

	// fontSize is in pixels.
	float scale = fontSize * sPixelScale(config) / FONT_BAKE_SIZE;
	if (scale <= 0.0f) {
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
		stbtt_GetBakedQuad(sGlyphs, EG_DRAWS_ATLAS_WIDTH, EG_DRAWS_ATLAS_HEIGHT, codepoint - FONT_FIRST_CHAR, &cursorX, &cursorY, &q, 1);

		float dx0 = q.x0 - startX;
		float dy0 = q.y0 - cursorY;
		float dx1 = q.x1 - startX;
		float dy1 = q.y1 - cursorY;
		q.x0      = startX + scale * dx0;
		q.y0      = cursorY - scale * dy0;
		q.x1      = startX + scale * dx1;
		q.y1      = cursorY - scale * dy1;

		sAddTextQuad(v, config, q.x0, q.y0, q.x1, q.y1, q.s0, q.t0, q.s1, q.t1);
	}
}

void verts_add_line(ecs_vec_t *v, const verts_config_t *config, float x1, float y1, float x2, float y2, float thickness)
{
	if (!sValid(v, config)) {
		return;
	}
	sAddLine(v, config, x1, y1, x2, y2, thickness);
}

void verts_add_point(ecs_vec_t *v, const verts_config_t *config, float size)
{
	if (!sValid(v, config)) {
		return;
	}
	if (size <= 0.0f) {
		return;
	}

	float scaledSize = size * sPixelScale(config);
	sAddSolidQuad(v, config, -scaledSize, -scaledSize, scaledSize, scaledSize);
}

void verts_add_circle(ecs_vec_t *v, const verts_config_t *config, float radius)
{
	if (!sValid(v, config)) {
		return;
	}
	if (radius <= 0.0f) {
		return;
	}

	int segments = 32;
	for (int i = 0; i < segments; ++i) {
		float angle0 = (float)i * (2.0f * EG_PI / (float)segments);
		float angle1 = (float)(i + 1) * (2.0f * EG_PI / (float)segments);
		sAddTriangle(v, config, 0.0f, 0.0f, cosf(angle0) * radius, sinf(angle0) * radius, cosf(angle1) * radius, sinf(angle1) * radius);
	}
}

void verts_add_circle_outline(ecs_vec_t *v, const verts_config_t *config, float radius, float thickness)
{
	if (!sValid(v, config)) {
		return;
	}
	if (radius <= 0.0f || thickness <= 0.0f) {
		return;
	}

	int   segments        = 32;
	float scaledThickness = thickness * sPixelScale(config);
	float innerRadius     = radius - scaledThickness * 0.5f;
	float outerRadius     = radius + scaledThickness * 0.5f;
	if (innerRadius < 0.0f) {
		innerRadius = 0.0f;
	}

	for (int i = 0; i < segments; ++i) {
		float angle0 = (float)i * (2.0f * EG_PI / (float)segments);
		float angle1 = (float)(i + 1) * (2.0f * EG_PI / (float)segments);

		float x0Outer = cosf(angle0) * outerRadius;
		float y0Outer = sinf(angle0) * outerRadius;
		float x1Outer = cosf(angle1) * outerRadius;
		float y1Outer = sinf(angle1) * outerRadius;
		float x0Inner = cosf(angle0) * innerRadius;
		float y0Inner = sinf(angle0) * innerRadius;
		float x1Inner = cosf(angle1) * innerRadius;
		float y1Inner = sinf(angle1) * innerRadius;

		sAddTriangle(v, config, x0Outer, y0Outer, x1Outer, y1Outer, x1Inner, y1Inner);
		sAddTriangle(v, config, x0Outer, y0Outer, x1Inner, y1Inner, x0Inner, y0Inner);
	}
}

// Adds one half-ring cap centred on (cx, cy) sweeping from startAngle to startAngle + PI.
static void sAddCap(ecs_vec_t *v, const verts_config_t *config, float cx, float cy, float startAngle, float radius, float innerRadius, int segments)
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

		sAddTriangle(v, config, ox0, oy0, ox1, oy1, ix1, iy1);
		sAddTriangle(v, config, ox0, oy0, ix1, iy1, ix0, iy0);
	}
}

void verts_add_capsule_outline(ecs_vec_t *v, const verts_config_t *config, float x1, float y1, float x2, float y2, float radius, float thickness)
{
	if (!sValid(v, config)) {
		return;
	}
	if (radius <= 0.0f || thickness <= 0.0f) {
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

	sAddLine(v, config, x1, y1, x2, y2, thickness);

	int   segments    = 24;
	float innerRadius = radius - thickness * sPixelScale(config) * 0.5f;
	sAddCap(v, config, centerX + nx * halfLength, centerY + ny * halfLength, EG_PI, radius, innerRadius, segments);
	sAddCap(v, config, centerX - nx * halfLength, centerY - ny * halfLength, 0.0f, radius, innerRadius, segments);
}

void verts_add_transform(ecs_vec_t *v, const verts_config_t *config, float scale)
{
	if (!sValid(v, config)) {
		return;
	}
	sAddLine(v, config, 0.0f, 0.0f, scale, 0.0f, 0.05f);
	sAddLine(v, config, 0.0f, 0.0f, 0.0f, scale, 0.05f);
}

void verts_add_rectangle(ecs_vec_t *v, const verts_config_t *config, float width, float height)
{
	if (!sValid(v, config)) {
		return;
	}
	if (width <= 0.0f || height <= 0.0f) {
		return;
	}

	sAddSolidQuad(v, config, -width * 0.5f, -height * 0.5f, width * 0.5f, height * 0.5f);
}

void verts_add_rectangle_outline(ecs_vec_t *v, const verts_config_t *config, float width, float height, float thickness)
{
	if (!sValid(v, config)) {
		return;
	}
	if (width <= 0.0f || height <= 0.0f || thickness <= 0.0f) {
		return;
	}

	float hw = width * 0.5f;
	float hh = height * 0.5f;
	sAddLine(v, config, -hw, -hh, hw, -hh, thickness);
	sAddLine(v, config, hw, -hh, hw, hh, thickness);
	sAddLine(v, config, hw, hh, -hw, hh, thickness);
	sAddLine(v, config, -hw, hh, -hw, -hh, thickness);
}

void verts_add_bounds(ecs_vec_t *v, const verts_config_t *config, float minX, float minY, float maxX, float maxY)
{
	if (!sValid(v, config)) {
		return;
	}
	sAddLine(v, config, minX, minY, maxX, minY, 0.05f);
	sAddLine(v, config, maxX, minY, maxX, maxY, 0.05f);
	sAddLine(v, config, maxX, maxY, minX, maxY, 0.05f);
	sAddLine(v, config, minX, maxY, minX, minY, 0.05f);
}

void verts_add_polygon(ecs_vec_t *v, const verts_config_t *config, const EgDraws_Vec2 *vertices, int vertexCount)
{
	if (!sValid(v, config)) {
		return;
	}
	if (vertices == NULL) {
		ecs_err("EgDraws: polygon vertices must not be null");
		return;
	}
	if (vertexCount < 3) {
		return;
	}

	for (int i = 1; i + 1 < vertexCount; ++i) {
		sAddTriangle(v, config, vertices[0].x, vertices[0].y, vertices[i].x, vertices[i].y, vertices[i + 1].x, vertices[i + 1].y);
	}
}
