#include "fd.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fanotify.h>
#include <sys/inotify.h>
#include <unistd.h>

int fd_fanotify_init()
{
	int fd = fanotify_init(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME, O_RDONLY);
	if (fd < 0) {
		perror("fanotify_init");
		return -1;
	}
	return fd;
}


int fd_close(int fd)
{
	int r = close(fd);
	return r;
}


int fd_close_valid(int fd)
{
	if (fd >= 0) {
		return close(fd);
	}
	return 0;
}

int fd_epoll_create()
{
	int fd = epoll_create1(0);
	if (fd < 0) {
		perror("epoll_create1");
		return fd;
	}
	return fd;
}

int fd_inotify_init1()
{
	int fd = inotify_init1(IN_NONBLOCK);
	if (fd < 0) {
		perror("inotify_init1");
		return fd;
	}
	return fd;
}