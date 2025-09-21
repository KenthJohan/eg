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

#define MARK_MASK (FAN_MODIFY | FAN_OPEN | FAN_EVENT_ON_CHILD | FAN_DELETE | FAN_MOVE | FAN_CLOSE | FAN_CREATE)

int fd_fanotify_mark_add(int fd, const char *pathname)
{
	int r = fanotify_mark(fd, FAN_MARK_ADD, MARK_MASK, AT_FDCWD, pathname);
	if (r < 0) {
		perror("fanotify_mark");
	}
	return r;
}

int fd_fanotify_mark_rm(int fd, const char *pathname)
{
	int r = fanotify_mark(fd, FAN_MARK_REMOVE, MARK_MASK, AT_FDCWD, pathname);
	if (r < 0) {
		perror("fanotify_mark");
	}
	return r;
}

int fd_read(int fd, void *buf, size_t count)
{
	ssize_t len = read(fd, buf, count);
	if (len < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("read");
		}
		return -1;
	}
	return (int)len;
}

#define MAX_EVENTS 8

void fd_epoll_ecs_wait(ecs_world_t *world, int epoll_fd, const ecs_map_t *map, ecs_id_t component, size_t size, const void *ptr)
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

static void info_header_print(ecs_world_t *world, ecs_entity_t event, ecs_entity_t entity, struct fanotify_event_metadata *metadata)
{
	int iterations = 0;
	struct fanotify_event_info_header *hdr = (struct fanotify_event_info_header *)(metadata + 1);
	while ((char *)hdr < (char *)metadata + metadata->event_len) {
		if (hdr->info_type == FAN_EVENT_INFO_TYPE_DFID_NAME) {
			struct fanotify_event_info_fid *fid_info = (struct fanotify_event_info_fid *)hdr;
			struct file_handle *fh = (struct file_handle *)fid_info->handle;
			char *file_name = fh->f_handle + fh->handle_bytes;
			iterations++;
			//printf("File name: %i %s %i\n", iterations, file_name, fid_info->hdr.info_type);
			ecs_enqueue(world, &(ecs_event_desc_t){.event = event, .entity = entity});
		}
		hdr = (struct fanotify_event_info_header *)((char *)hdr + hdr->len);
	}
}

static struct {
	uint64_t value;
	const char *name;
} fanotify_events[] = {
{FAN_ACCESS, "FAN_ACCESS"},
{FAN_MODIFY, "FAN_MODIFY"},
{FAN_ATTRIB, "FAN_ATTRIB"},
{FAN_CLOSE_WRITE, "FAN_CLOSE_WRITE"},
{FAN_CLOSE_NOWRITE, "FAN_CLOSE_NOWRITE"},
{FAN_OPEN, "FAN_OPEN"},
{FAN_MOVED_FROM, "FAN_MOVED_FROM"},
{FAN_MOVED_TO, "FAN_MOVED_TO"},
{FAN_CREATE, "FAN_CREATE"},
{FAN_DELETE, "FAN_DELETE"},
{FAN_DELETE_SELF, "FAN_DELETE_SELF"},
{FAN_MOVE_SELF, "FAN_MOVE_SELF"},
{FAN_OPEN_EXEC, "FAN_OPEN_EXEC"},
{FAN_Q_OVERFLOW, "FAN_Q_OVERFLOW"},
{FAN_FS_ERROR, "FAN_FS_ERROR"},
{FAN_OPEN_PERM, "FAN_OPEN_PERM"},
{FAN_ACCESS_PERM, "FAN_ACCESS_PERM"},
{FAN_OPEN_EXEC_PERM, "FAN_OPEN_EXEC_PERM"},
/* FAN_DIR_MODIFY is deprecated and reserved */
//{FAN_PRE_ACCESS,         "FAN_PRE_ACCESS"},
//{FAN_MNT_ATTACH,         "FAN_MNT_ATTACH"},
//{FAN_MNT_DETACH,         "FAN_MNT_DETACH"},
{FAN_EVENT_ON_CHILD, "FAN_EVENT_ON_CHILD"},
{FAN_RENAME, "FAN_RENAME"},
{FAN_ONDIR, "FAN_ONDIR"},
// Add any additional flags as needed from your kernel headers
};

void print_fanotify_mask(uint64_t mask)
{
	printf("fanotify_event_metadata.mask: 0x%lx\n", mask);
	for (size_t i = 0; i < sizeof(fanotify_events) / sizeof(fanotify_events[0]); ++i) {
		if (mask & fanotify_events[i].value) {
			printf("  %s\n", fanotify_events[i].name);
		}
	}
}

void handle_notifications(ecs_world_t *world, ecs_entity_t event, ecs_entity_t entity, char *buffer, int len)
{
	struct fanotify_event_metadata *metadata = (struct fanotify_event_metadata *)buffer;
	/*
	struct fanotify_event_info_header *info;
	struct fanotify_event_info_error *err;
	struct fanotify_event_info_fid *fid;
	*/
	for (; FAN_EVENT_OK(metadata, len); metadata = FAN_EVENT_NEXT(metadata, len)) {
		if (metadata->fd == FAN_NOFD) {
			//print_fanotify_mask(metadata->mask);
			if (metadata->mask & FAN_MODIFY) {
				info_header_print(world, event, entity, metadata);
			}
		}
	}
}
