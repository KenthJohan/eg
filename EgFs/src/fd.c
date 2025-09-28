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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "EgFs.h"

/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/


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

int fd_inotify_add(int fd, char const *path, uint32_t mask)
{
	int r = inotify_add_watch(fd, path, mask);
	if (r < 0) {
		perror("inotify_add_watch");
	}
	return r;
}

int fd_inotify_rm(int fd, int wd)
{
	int r = inotify_rm_watch(fd, wd);
	if (r < 0) {
		perror("inotify_rm_watch");
	}
	return r;
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
			printf("File name: %i %s %i\n", iterations, file_name, fid_info->hdr.info_type);
			// ecs_enqueue(world, &(ecs_event_desc_t){.event = event, .entity = entity});

			/*
			char fullpath[1024];
			if (path[0] == '.') {
			    snprintf(fullpath, sizeof(fullpath), "%s%s", "$CWD", path + 1);
			} else {
			    snprintf(fullpath, sizeof(fullpath), "%s", path);
			}
			printf("fullpath = '%s'\n", fullpath);
			ecs_entity_t e = ecs_entity_init(world, &(ecs_entity_desc_t){ .name = fullpath, .sep = "/", .parent = EgFsFiles });
			*/
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

void fan_handle_notifications(ecs_world_t *world, ecs_entity_t event, ecs_entity_t entity, uint32_t mask, char *buffer, int len)
{
	struct fanotify_event_metadata *metadata = (struct fanotify_event_metadata *)buffer;
	/*
	struct fanotify_event_info_header *info;
	struct fanotify_event_info_error *err;
	struct fanotify_event_info_fid *fid;
	*/
	for (; FAN_EVENT_OK(metadata, len); metadata = FAN_EVENT_NEXT(metadata, len)) {
		if (metadata->fd == FAN_NOFD) {
			// print_fanotify_mask(metadata->mask);
			//  FAN_MODIFY
			if (metadata->mask & mask) {
				info_header_print(world, event, entity, metadata);
			}
		}
	}
}

/* Display information from inotify_event structure */
void displayInotifyEvent(struct inotify_event *i)
{
	printf("    wd =%2d; ", i->wd);
	if (i->cookie > 0)
		printf("cookie =%4d; ", i->cookie);

	printf("mask = ");
	if (i->mask & IN_ACCESS)
		printf("IN_ACCESS ");
	if (i->mask & IN_ATTRIB)
		printf("IN_ATTRIB ");
	if (i->mask & IN_CLOSE_NOWRITE)
		printf("IN_CLOSE_NOWRITE ");
	if (i->mask & IN_CLOSE_WRITE)
		printf("IN_CLOSE_WRITE ");
	if (i->mask & IN_CREATE)
		printf("IN_CREATE ");
	if (i->mask & IN_DELETE)
		printf("IN_DELETE ");
	if (i->mask & IN_DELETE_SELF)
		printf("IN_DELETE_SELF ");
	if (i->mask & IN_IGNORED)
		printf("IN_IGNORED ");
	if (i->mask & IN_ISDIR)
		printf("IN_ISDIR ");
	if (i->mask & IN_MODIFY)
		printf("IN_MODIFY ");
	if (i->mask & IN_MOVE_SELF)
		printf("IN_MOVE_SELF ");
	if (i->mask & IN_MOVED_FROM)
		printf("IN_MOVED_FROM ");
	if (i->mask & IN_MOVED_TO)
		printf("IN_MOVED_TO ");
	if (i->mask & IN_OPEN)
		printf("IN_OPEN ");
	if (i->mask & IN_Q_OVERFLOW)
		printf("IN_Q_OVERFLOW ");
	if (i->mask & IN_UNMOUNT)
		printf("IN_UNMOUNT ");
	printf("\n");
	if (i->len > 0)
		printf("        name = %s\n", i->name);
}

void fd_handle_inotify_events(ecs_world_t *world, ecs_entity_t event, ecs_entity_t parent, uint32_t mask, ecs_map_t *map, char *buffer, int len)
{
	struct inotify_event *i;
	for (char *p = buffer; p < buffer + len; p += sizeof(struct inotify_event) + i->len) {
		i = (struct inotify_event *)p;
		if (i->len <= 0) {
			continue;
		}
		if ((i->mask & mask) == 0) {
			continue;
		}
		//displayInotifyEvent(i);
 		ecs_entity_t *e = ecs_map_get(map, i->wd);
		if (e == NULL) {
			continue;
		} 
		EgFsWatch *w = ecs_get(world, *e, EgFsWatch);
		char const * parent_path = ecs_get_name(world, w->file);

		//ecs_get_path_w_sep(world, 0, w->file, NULL); a a
		char fullpath[1024];
		char const * name = i->name;
		snprintf(fullpath, sizeof(fullpath), "$CWD/%s/%s", parent_path, name);
		printf("fullpath = '%s'\n", fullpath);
		ecs_entity_t e1 = ecs_entity_init(world, &(ecs_entity_desc_t){.name = fullpath, .sep = "/", .parent = parent});
		ecs_enqueue(world, &(ecs_event_desc_t){.event = event, .entity = e1});
	}
}



int fd_create_udp_socket(const char *ip, int port)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (sockfd < 0) {
		perror("socket");
		return sockfd;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if (ip == NULL) {
		servaddr.sin_addr.s_addr = INADDR_ANY;
	} else {
		servaddr.sin_addr.s_addr = inet_addr(ip);
	}
	servaddr.sin_port = htons(port);

	if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind");
		close(sockfd);
		return -1;
	}

	return sockfd;
}
