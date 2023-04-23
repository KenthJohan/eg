/*
https://developer.ibm.com/tutorials/l-ubuntu-inotify/
*/

#include "eg_dirwatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 ∗ ( EVENT_SIZE + 16 ) )



typedef struct
{
	eg_fs_monitor_t public;
	int fd;
	int wd;
} eg_fs_monitor_linux_t;



int eg_fs_monitor_size()
{
	return sizeof(eg_fs_monitor_linux_t);
}



eg_fs_monitor_t * eg_fs_monitor_init(char const * path, void * memory)
{
	eg_fs_monitor_linux_t * ptr = (eg_fs_monitor_linux_t *)memory;
	ptr->fd = inotify_init();
	ptr->wd = inotify_add_watch( fd, "/home/strike", IN_MODIFY | IN_CREATE | IN_DELETE );
	return (eg_fs_monitor_t*)ptr;
}


int eg_fs_wait_event(eg_fs_monitor_t * monitor, int32_t wait_ms, char out_path[EG_FS_PATH_LENGTH])
{
	assert(monitor);
	assert(out_path);
	eg_fs_monitor_win32_t * mon = (eg_fs_monitor_win32_t *)monitor;
	return 1;
}

