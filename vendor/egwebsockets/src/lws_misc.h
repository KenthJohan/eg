#pragma once

#include <libwebsockets.h>

char const *lws_callback_reasons_tostr(enum lws_callback_reasons reason);