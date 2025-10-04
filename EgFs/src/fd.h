#pragma once

#include <flecs.h>

int fd_read(int fd, void *buf, size_t count);

int fd_close(int fd);

int fd_close_valid(int fd);

/*
The inotify API provides a mechanism for monitoring filesystem
events.  Inotify can be used to monitor individual files, or to
monitor directories.  When a directory is monitored, inotify will
return events for the directory itself, and for files inside the
directory.

The following system calls are used with this API:

•  inotify_init(2) creates an inotify instance and returns a file
    descriptor referring to the inotify instance.  The more recent
    inotify_init1(2) is like inotify_init(2), but has a flags
    argument that provides access to some extra functionality.

•  inotify_add_watch(2) manipulates the "watch list" associated
    with an inotify instance.  Each item ("watch") in the watch
    list specifies the pathname of a file or directory, along with
    some set of events that the kernel should monitor for the file
    referred to by that pathname.  inotify_add_watch(2) either
    creates a new watch item, or modifies an existing watch.  Each
    watch has a unique "watch descriptor", an integer returned by
    inotify_add_watch(2) when the watch is created.

•  When events occur for monitored files and directories, those
    events are made available to the application as structured data
    that can be read from the inotify file descriptor using read(2)
    (see below).

•  inotify_rm_watch(2) removes an item from an inotify watch list.

•  When all file descriptors referring to an inotify instance have
    been closed (using close(2)), the underlying object and its
    resources are freed for reuse by the kernel; all associated
    watches are automatically freed.
*/

#define FD_IN_ACCESS        0x00000001                          /* File was accessed.  */
#define FD_IN_MODIFY        0x00000002                          /* File was modified.  */
#define FD_IN_ATTRIB        0x00000004                          /* Metadata changed.  */
#define FD_IN_CLOSE_WRITE   0x00000008                          /* Writtable file was closed.  */
#define FD_IN_CLOSE_NOWRITE 0x00000010                          /* Unwrittable file closed.  */
#define FD_IN_CLOSE         (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) /* Close.  */
#define FD_IN_OPEN          0x00000020                          /* File was opened.  */
#define FD_IN_MOVED_FROM    0x00000040                          /* File was moved from X.  */
#define FD_IN_MOVED_TO      0x00000080                          /* File was moved to Y.  */
#define FD_IN_MOVE          (IN_MOVED_FROM | IN_MOVED_TO)       /* Moves.  */
#define FD_IN_CREATE        0x00000100                          /* Subfile was created.  */
#define FD_IN_DELETE        0x00000200                          /* Subfile was deleted.  */
#define FD_IN_DELETE_SELF   0x00000400                          /* Self was deleted.  */
#define FD_IN_MOVE_SELF     0x00000800                          /* Self was moved.  */
#define FD_IN_UNMOUNT       0x00002000                          /* Backing fs was unmounted.  */
#define FD_IN_Q_OVERFLOW    0x00004000                          /* Event queued overflowed.  */
#define FD_IN_IGNORED       0x00008000                          /* File was ignored.  */
#define FD_IN_CLOSE         (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) /* Close.  */
#define FD_IN_MOVE          (IN_MOVED_FROM | IN_MOVED_TO)       /* Moves.  */
#define FD_IN_ONLYDIR       0x01000000                          /* Only watch the path if it is adirectory.  */
#define FD_IN_DONT_FOLLOW   0x02000000                          /* Do not follow a sym link.  */
#define FD_IN_EXCL_UNLINK   0x04000000                          /* Exclude events on unlinkedobjects.  */
#define FD_IN_MASK_CREATE   0x10000000                          /* Only create watches.  */
#define FD_IN_MASK_ADD      0x20000000                          /* Add to the mask of an alreadyexisting watch.  */
#define FD_IN_ISDIR         0x40000000                          /* Event occurred against dir.  */
#define FD_IN_ONESHOT       0x80000000                          /* Only send event once.  */
#define FD_IN_ALL_EVENTS    (FD_IN_ACCESS | FD_IN_MODIFY | FD_IN_ATTRIB | FD_IN_CLOSE_WRITE | FD_IN_CLOSE_NOWRITE | FD_IN_OPEN | FD_IN_MOVED_FROM | FD_IN_MOVED_TO | FD_IN_CREATE | FD_IN_DELETE | FD_IN_DELETE_SELF | FD_IN_MOVE_SELF)

typedef struct
{
	int wd;              /* Watch descriptor.  */
	uint32_t mask;       /* Watch mask.  */
	uint32_t cookie;     /* Cookie to synchronize two events.  */
	uint32_t len;        /* Length (including NULs) of name.  */
	char name __flexarr; /* Name.  */
} fd_inotify_event;

int fd_inotify_init1();

int fd_inotify_add(int fd, char const *path, uint32_t mask);

int fd_inotify_rm(int fd, int wd);

void fd_handle_inotify_events(ecs_world_t *world, ecs_entity_t event, ecs_entity_t parent, uint32_t mask, ecs_map_t *map, char *buffer, int len);

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

#define FD_FAN_ACCESS        0x00000001 /* File was accessed */
#define FD_FAN_MODIFY        0x00000002 /* File was modified */
#define FD_FAN_ATTRIB        0x00000004 /* Metadata changed */
#define FD_FAN_CLOSE_WRITE   0x00000008 /* Writtable file closed */
#define FD_FAN_CLOSE_NOWRITE 0x00000010 /* Unwrittable file closed */
#define FD_FAN_OPEN          0x00000020 /* File was opened */
#define FD_FAN_MOVED_FROM    0x00000040 /* File was moved from X */
#define FD_FAN_MOVED_TO      0x00000080 /* File was moved to Y */
#define FD_FAN_CREATE        0x00000100 /* Subfile was created */
#define FD_FAN_DELETE        0x00000200 /* Subfile was deleted */
#define FD_FAN_DELETE_SELF   0x00000400 /* Self was deleted */
#define FD_FAN_MOVE_SELF     0x00000800 /* Self was moved */
#define FD_FAN_OPEN_EXEC     0x00001000 /* File was opened for exec */

int fd_fanotify_init();

int fd_fanotify_mark_add(int fd, const char *pathname);

int fd_fanotify_mark_rm(int fd, const char *pathname);

void fan_handle_notifications(ecs_world_t *world, ecs_entity_t event, ecs_entity_t entity, uint32_t mask, char *buffer, int len);

/*
The epoll API performs a similar task to poll(2): monitoring
       multiple file descriptors to see if I/O is possible on any of
       them.  The epoll API can be used either as an edge-triggered or a
       level-triggered interface and scales well to large numbers of
       watched file descriptors.

       The central concept of the epoll API is the epoll instance, an in-
       kernel data structure which, from a user-space perspective, can be
       considered as a container for two lists:

       •  The interest list (sometimes also called the epoll set): the
          set of file descriptors that the process has registered an
          interest in monitoring.

       •  The ready list: the set of file descriptors that are "ready"
          for I/O.  The ready list is a subset of (or, more precisely, a
          set of references to) the file descriptors in the interest
          list.  The ready list is dynamically populated by the kernel as
          a result of I/O activity on those file descriptors.

       The following system calls are provided to create and manage an
       epoll instance:

       •  epoll_create(2) creates a new epoll instance and returns a file
          descriptor referring to that instance.  (The more recent
          epoll_create1(2) extends the functionality of epoll_create(2).)

       •  Interest in particular file descriptors is then registered via
          epoll_ctl(2), which adds items to the interest list of the
          epoll instance.

       •  epoll_wait(2) waits for I/O events, blocking the calling thread
          if no events are currently available.  (This system call can be
          thought of as fetching items from the ready list of the epoll
          instance.)
*/

int fd_epoll_create();

// Returns 0 in case of success
int fd_epoll_add(int epoll_fd, int fd);

// Returns 0 in case of success
int fd_epoll_rm(int epoll_fd, int fd);

void fd_epoll_ecs_wait(ecs_world_t *world, int epoll_fd, const ecs_map_t *map, ecs_id_t component, size_t size, const void *ptr);

int fd_create_udp_socket(const char *ip, int port);

int test_inotify(char *path);

void fd_udp_test_recv_send(int sockfd);
