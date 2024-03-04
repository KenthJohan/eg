#pragma once

#include <libwebsockets.h>
#include "wstypes.h"

int spam_start(struct per_vhost_data__minimal *vhd);
int spam_fini(struct per_vhost_data__minimal *vhd);