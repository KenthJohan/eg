#pragma once
#include <stdarg.h>
#include <stdint.h>
#include "flecs.h"

#ifdef __cplusplus
extern "C" {
#endif


void eg_iter_info(ecs_iter_t *it, const char *fname, const char *file, int32_t line);



#define EG_ITER_INFO(it) eg_iter_info((it), __FUNCTION__, __FILE__, __LINE__);




#ifdef __cplusplus
}
#endif
