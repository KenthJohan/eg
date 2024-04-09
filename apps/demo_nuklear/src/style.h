#pragma once
#include <nuklear.h>

enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK};

void
set_style(struct nk_context *ctx, enum theme theme);