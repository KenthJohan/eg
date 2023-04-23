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
} _eg_dirwatch_t;


void _eg_dirwatch_free(_eg_dirwatch_t * dirwatch)
{
	ecs_os_free(dirwatch);
}




_eg_dirwatch_t * _eg_dirwatch_init(eg_dirwatch_desc_t * desc)
{
	(void)desc;
	_eg_dirwatch_t * dirwatch = ecs_os_calloc_t(_eg_dirwatch_t);
	dirwatch->fd = inotify_init();
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		exit(2);
	}
	return dirwatch;
}




void _eg_dirwatch_add(_eg_dirwatch_t * dirwatch, char const * path)
{
	/* Step 2. Add Watch */
	int wd = inotify_add_watch(dirwatch->fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);

	if(wd==-1)
	{
		printf("Could not watch : %s\n",path);
	}
	else
	{
		printf("Watching : %s\n",path);
	}
}



int _eg_dirwatch_pull(_eg_dirwatch_t * dirwatch, int32_t timeout_ms, char out_path[EG_DIRWATCH_PATH_LENGTH])
{
	return 1;
}








void eg_dirwatch_fini(eg_dirwatch_t * dirwatch)
{
	_eg_dirwatch_fini((_eg_dirwatch_t*)dirwatch);
}

eg_dirwatch_t * eg_dirwatch_init(eg_dirwatch_desc_t * desc)
{
	return (eg_dirwatch_t*)_eg_dirwatch_init(desc);
}

void eg_dirwatch_add(eg_dirwatch_t * dirwatch, char const * path)
{
	_eg_dirwatch_add((_eg_dirwatch_t*)dirwatch, path);
}


int eg_dirwatch_pull(eg_dirwatch_t * dirwatch, int32_t timeout_ms, char out_path[EG_DIRWATCH_PATH_LENGTH])
{
	return _eg_dirwatch_pull((_eg_dirwatch_t*)dirwatch, timeout_ms, out_path);
}