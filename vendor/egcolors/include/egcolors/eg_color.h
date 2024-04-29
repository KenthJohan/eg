#pragma once
#include <stdint.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} eg_color_rgb_t;

typedef struct
{
    uint8_t h;
    uint8_t s;
    uint8_t v;
} eg_color_hsv_t;

void eg_color_hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t *out_r, uint8_t *out_g, uint8_t *out_b);
void eg_color_rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, uint8_t *out_h, uint8_t *out_s, uint8_t *out_v);