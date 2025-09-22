#pragma once

#include <flecs.h>

#define FD_FAN_ACCESS 0x00000001        /* File was accessed */
#define FD_FAN_MODIFY 0x00000002        /* File was modified */
#define FD_FAN_ATTRIB 0x00000004        /* Metadata changed */
#define FD_FAN_CLOSE_WRITE 0x00000008   /* Writtable file closed */
#define FD_FAN_CLOSE_NOWRITE 0x00000010 /* Unwrittable file closed */
#define FD_FAN_OPEN 0x00000020          /* File was opened */
#define FD_FAN_MOVED_FROM 0x00000040    /* File was moved from X */
#define FD_FAN_MOVED_TO 0x00000080      /* File was moved to Y */
#define FD_FAN_CREATE 0x00000100        /* Subfile was created */
#define FD_FAN_DELETE 0x00000200        /* Subfile was deleted */
#define FD_FAN_DELETE_SELF 0x00000400   /* Self was deleted */
#define FD_FAN_MOVE_SELF 0x00000800     /* Self was moved */
#define FD_FAN_OPEN_EXEC 0x00001000     /* File was opened for exec */

/*
The fanotify API provides notification and interception of
    filesystem events.  Use cases include virus scanning and
    hierarchical storage management.  In the original fanotify API,
    only a limited set of events was supported.  In particular, there
    was no support for create, delete, and move events.  The support
    for those events was added in Linux 5.1.  (See inotify(7) for
    details of an API that did notify those events pre Linux 5.1.)

    Additional capabilities compared to the inotify(7) API include the
    ability to monitor all of the objects in a mounted filesystem, the
    ability to make access permission decisions, and the possibility
    to read or modify files before access by other applications.

    The following system calls are used with this API:
    fanotify_init(2), fanotify_mark(2), read(2), write(2), and
    close(2).
*/
int fd_fanotify_init();

int fd_close(int fd);

int fd_close_valid(int fd);

int fd_epoll_create();

int fd_inotify_init1();

int fd_epoll_add(int epoll_fd, int fd);

int fd_epoll_rm(int epoll_fd, int fd);

int fd_fanotify_mark_add(int fd, const char *pathname);

int fd_fanotify_mark_rm(int fd, const char *pathname);

void fd_epoll_ecs_wait(ecs_world_t *world, int epoll_fd, const ecs_map_t *map, ecs_id_t component, size_t size, const void *ptr);

int fd_read(int fd, void *buf, size_t count);

void handle_notifications(ecs_world_t *world, ecs_entity_t event, ecs_entity_t entity, uint32_t mask, char *buffer, int len);