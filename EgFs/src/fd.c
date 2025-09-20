#define _GNU_SOURCE
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
#include <sys/epoll.h>
#include <errno.h>

int fd_fanotify_init()
{
	int fd = fanotify_init(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME | FAN_REPORT_FID | FAN_NONBLOCK, O_RDONLY);
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

int fd_epoll_add(int epoll_fd, int fd)
{
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fd;
	int r = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
	if (r < 0) {
		perror("epoll_ctl EPOLL_CTL_ADD");
	}
	return r;
}

int fd_epoll_rm(int epoll_fd, int fd)
{
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = fd;
	int r = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
	if (r < 0) {
		perror("epoll_ctl EPOLL_CTL_DEL");
	}
	return r;
}

int fd_fanotify_mark_add(int fd, const char *pathname)
{
	int r = fanotify_mark(fd, FAN_MARK_ADD, FAN_MODIFY | FAN_OPEN | FAN_EVENT_ON_CHILD, AT_FDCWD, pathname);
	if (r < 0) {
		perror("fanotify_mark");
	}
	return r;
}

int fd_fanotify_mark_rm(int fd, const char *pathname)
{
	int r = fanotify_mark(fd, FAN_MARK_REMOVE, FAN_MODIFY | FAN_OPEN | FAN_EVENT_ON_CHILD, AT_FDCWD, pathname);
	if (r < 0) {
		perror("fanotify_mark");
	}
	return r;
}


#define MAX_EVENTS 10

void fd_epoll_ecs_wait(ecs_world_t *world, int epoll_fd, const ecs_map_t *map, ecs_id_t component,size_t size,const void *ptr)
{
	struct epoll_event events[MAX_EVENTS];
	int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
	if (nfds == -1) {
		if (errno == EINTR) {
			return;
		}
		perror("epoll_wait");
		return;
	}

	for (int j = 0; j < nfds; ++j) {
		ecs_entity_t *e = ecs_map_get(map, events[j].data.fd);
		if (e == NULL) {
			continue;
		}
		ecs_trace("Epoll event on fd=%d for entity '%s'", events[j].data.fd, ecs_get_name(world, *e));
		ecs_set_id(world, *e, component, size, ptr);
	}
}







static void info_header_print(struct fanotify_event_metadata *metadata, char *buf, int len)
{
	char path[PATH_MAX];

	struct fanotify_event_info_header *hdr = (struct fanotify_event_info_header *)(metadata + 1);
	while ((char *)hdr < (char *)metadata + metadata->event_len) {
		if (hdr->info_type == FAN_EVENT_INFO_TYPE_DFID_NAME) {
			struct fanotify_event_info_fid *fid_info = (struct fanotify_event_info_fid *)hdr;
			struct file_handle *fh = (struct file_handle *)fid_info->handle;
			char *file_name = fh->f_handle + fh->handle_bytes;
			printf("File name: %s\n", file_name);
		}
		hdr = (struct fanotify_event_info_header *)((char *)hdr + hdr->len);
	}
}

static void handle_notifications(char *buffer, int len)
{
	struct fanotify_event_metadata *event = (struct fanotify_event_metadata *)buffer;
	/*
	struct fanotify_event_info_header *info;
	struct fanotify_event_info_error *err;
	struct fanotify_event_info_fid *fid;
	*/
	for (; FAN_EVENT_OK(event, len); event = FAN_EVENT_NEXT(event, len)) {
		if (event->fd == FAN_NOFD) {
			info_header_print(event, buffer, len);
		}
	}
}

void handle_fanotify_response(ecs_world_t *world, int fd)
{
	// Should be large enough to hold at least one full fanotify event and its associated info records. 
	// The kernel will return as many events as fit in the buffer, 
	// but you might not get all pending events if your buffer is too small.
	char buf[4096];
	ssize_t len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		perror("read from fanotify fd");
		return;
	}
	handle_notifications(buf, len);
}




