#include "egcolors/eg_color.h"

void eg_color_hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t *out_r, uint8_t *out_g, uint8_t *out_b)
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

void eg_color_rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, uint8_t *out_h, uint8_t *out_s, uint8_t *out_v)
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