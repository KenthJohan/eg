#pragma once
#include "wstypes.h"

eglws_vhd_t * eglws_vhd_init(struct lws *wsi, void *in);

void eglws_vhd_fini(eglws_vhd_t * vhd);

int eglws_vhd_consume(eglws_vhd_t * vhd, eglws_pss_t * pss, struct lws *wsi);

int eglws_vhd_broadcast(eglws_vhd_t * vhd, void * in, int len);

int eglws_vhd_send_message(eglws_vhd_t * vhd, void const * data, int len);