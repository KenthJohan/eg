/*
https://wiki.libsdl.org/SDL3/SDL_ReadIO
https://github.com/SanderMertens/flecs/blob/master/examples/c/entities/hooks/src/main.c
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/datastructures/vec.c#L118
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/include/flecs/datastructures/vec.h
https://github.com/SanderMertens/flecs/blob/733591da5682cea01857ecf2316ff6a635f4289d/src/addons/alerts.c#L39
https://github.com/libsdl-org/SDL/blob/0fcaf47658be96816a851028af3e73256363a390/test/testautomation_iostream.c#L477
*/

// Define _GNU_SOURCE, Otherwise we don't get O_LARGEFILE
#define _GNU_SOURCE

#include "EgFs.h"
#include "EgFs/EgFsPath.h"
#include "EgFs/EgFsEpoll.h"

#include <stdlib.h>
#include <stdio.h>
#include <ecsx.h>
#include <egmisc.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <sys/fanotify.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/fanotify.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysmacros.h>

#ifndef FILEID_INO32_GEN
#define FILEID_INO32_GEN 1
#endif
#ifndef FILEID_INVALID
#define FILEID_INVALID 0xff
#endif

#include "fd.h"

ECS_COMPONENT_DECLARE(EgFsEpollFd);

/*
The epoll API performs a similar task to poll(2): monitoring
multiple file descriptors to see if I/O is possible on any of
them.  The epoll API can be used either as an edge-triggered or a
level-triggered interface and scales well to large numbers of
watched file descriptors.

The destructor should free resources.
The move hook should move resources from one location to another.
*/

ECS_CTOR(EgFsEpollFd, ptr, {
	ptr->fd = fd_epoll_create();
	ecs_map_init(&ptr->map, NULL);
})

ECS_DTOR(EgFsEpollFd, ptr, {
	fd_close_valid(ptr->fd);
	ecs_map_fini(&ptr->map);
})

ECS_MOVE(EgFsEpollFd, dst, src, {
	fd_close_valid(dst->fd);
	dst->fd = src->fd;
	src->fd = -1; // Invalidate the source fd

	ecs_map_fini(&dst->map);
	dst->map = src->map;
	src->map = (ecs_map_t){0};
})

#define MAX_EVENTS 10
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + NAME_MAX + 1))

static void enqueue_inotify_event(ecs_world_t *world, int fd)
{
	char buffer[BUF_LEN];
	ssize_t len = read(fd, buffer, BUF_LEN);
	if (len < 0 && errno != EAGAIN) {
		perror("read");
		return;
	}
	ssize_t pos = 0;
	while (pos < len) {
		struct inotify_event *ev = (struct inotify_event *)&buffer[pos];
		printf("Event: wd=%d mask=0x%x cookie=%u len=%u, ", ev->wd, ev->mask, ev->cookie, ev->len);
		if (ev->len > 0)
			printf(" name=%s", ev->name);
		printf("\n");
		pos += EVENT_SIZE + ev->len;
	}
}

int open_mount_by_fsid(__kernel_fsid_t fsid, char *mount_path, size_t mount_path_len)
{
	FILE *fp = fopen("/proc/self/mountinfo", "r");
	if (!fp)
		return -1;

	char line[PATH_MAX * 2];
	int found = 0;
	while (fgets(line, sizeof(line), fp)) {
		// mountinfo fields:
		// 1: mount ID
		// 2: parent ID
		// 3: major:minor
		// 4: root
		// 5: mount point
		char *saveptr = NULL;
		char *token = strtok_r(line, " ", &saveptr); // mount ID
		token = strtok_r(NULL, " ", &saveptr);       // parent ID
		token = strtok_r(NULL, " ", &saveptr);       // major:minor
		unsigned int major = 0, minor = 0;
		if (sscanf(token, "%u:%u", &major, &minor) != 2)
			continue;
		dev_t dev = makedev(major, minor);

		token = strtok_r(NULL, " ", &saveptr); // root
		token = strtok_r(NULL, " ", &saveptr); // mount point
		char *mountpoint = token;

		if (fsid.val[0] == dev) {
			strncpy(mount_path, mountpoint, mount_path_len - 1);
			mount_path[mount_path_len - 1] = '\0';
			found = 1;
			break;
		}
	}
	fclose(fp);
	if (!found)
		return -1;
	int fd = open(mount_path, O_PATH);
	return fd;
}

#define FANOTIFY_BUF_SIZE 4096

static void enqueue_fanotify_event(ecs_world_t *world, int fd)
{
	char buf[FANOTIFY_BUF_SIZE];
	ssize_t len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		perror("read from fanotify fd");
		return;
	}

	struct fanotify_event_metadata *metadata;
	for (metadata = (struct fanotify_event_metadata *)buf;
	     FAN_EVENT_OK(metadata, len);
	     metadata = FAN_EVENT_NEXT(metadata, len)) {

		if (metadata->fd >= 0) {
			char path[PATH_MAX];
			snprintf(path, sizeof(path), "/proc/self/fd/%d", metadata->fd);
			ssize_t path_len = readlink(path, path, sizeof(path) - 1);
			if (path_len > 0)
				path[path_len] = '\0';
			else
				strcpy(path, "unknown");

			printf("Event: ");
			if (metadata->mask & FAN_CREATE)
				printf("CREATE ");
			if (metadata->mask & FAN_DELETE)
				printf("DELETE ");
			if (metadata->mask & FAN_MODIFY)
				printf("MODIFY ");
			if (metadata->mask & FAN_ACCESS)
				printf("ACCESS ");
			printf("on %s\n", path);

			close(metadata->fd);
		}
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
			char mount_path[PATH_MAX] = {0};
			int mountfd = open_mount_by_fsid(fid_info->fsid, mount_path, sizeof(mount_path));
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
	char buf[FANOTIFY_BUF_SIZE];
	ssize_t len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		perror("read from fanotify fd");
		return;
	}
	handle_notifications(buf, len);
}

static void System_epoll(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgFsEpollFd *o = ecs_field(it, EgFsEpollFd, 0); // self
	for (int i = 0; i < it->count; ++i, ++o) {
		struct epoll_event events[MAX_EVENTS];
		int nfds = epoll_wait(o->fd, events, MAX_EVENTS, 100);
		if (nfds == -1) {
			if (errno == EINTR)
				continue;
			perror("epoll_wait");
			break;
		}

		for (int j = 0; j < nfds; ++j) {
			ecs_entity_t *e = ecs_map_get(&o->map, events[j].data.fd);
			if (e == NULL) {
				continue;
			}
			if (ecs_has(world, *e, EgFsInotifyFd)) {
				enqueue_inotify_event(world, events[j].data.fd);
			}
			if (ecs_has(world, *e, EgFsFanotifyFd)) {
				handle_fanotify_response(world, events[j].data.fd);
			}
		}
	} // END FOR LOOP
}

void EgFsEpollImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFsEpoll);
	ecs_set_name_prefix(world, "EgFsEpoll");

	ECS_COMPONENT_DEFINE(world, EgFsEpollFd);

	ecs_set_hooks_id(world, ecs_id(EgFsEpollFd),
	&(ecs_type_hooks_t){
	.flags = ECS_TYPE_HOOK_COPY_ILLEGAL,
	.move = ecs_move(EgFsEpollFd),
	.dtor = ecs_dtor(EgFsEpollFd),
	.ctor = ecs_ctor(EgFsEpollFd),
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsEpollFd),
	.members = {
	{.name = "fd", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_epoll", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_epoll,
	.query.terms =
	{
	{.id = ecs_id(EgFsEpollFd), .src.id = EcsSelf},
	}});
}
