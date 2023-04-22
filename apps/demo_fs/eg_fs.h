#pragma once
#include <stddef.h>
#include <stdint.h>


#define EG_FS_PATH_LENGTH 1024

typedef struct
{
	int dummy;
} eg_fs_monitor_t;




int eg_fs_monitor_size();

eg_fs_monitor_t * eg_fs_monitor_init(char const * path, void * memory);

int eg_fs_wait_event(eg_fs_monitor_t * monitor, int32_t wait_ms, char out_path[EG_FS_PATH_LENGTH]);