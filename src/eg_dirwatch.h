#pragma once
#include <stddef.h>
#include <stdint.h>


#define EG_DIRWATCH_PATH_LENGTH 1024

typedef struct
{
	int dummy;
} eg_dirwatch_t;




int eg_dirwatch_size();
void eg_dirwatch_init(eg_dirwatch_t * dirwatch);
void eg_dirwatch_add(eg_dirwatch_t * dirwatch, char const * path);
int eg_dirwatch_pull(eg_dirwatch_t * dirwatch, int32_t timeout_ms, char out_path[EG_DIRWATCH_PATH_LENGTH]);