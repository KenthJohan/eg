#include "sokol_source.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_DEBUGTEXT_IMPL
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_fetch.h"
#include "sokol/util/sokol_debugtext.h"

#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
