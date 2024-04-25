#define _GNU_SOURCE /* To get defns of NI_MAXSERV and NI_MAXHOST */

#include "egcan.h"
#include <egquantities.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/epoll.h>
#include <errno.h>
#include <assert.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

ECS_COMPONENT_DECLARE(EgCanEpoll);
ECS_COMPONENT_DECLARE(EgCanBusDescription);
ECS_COMPONENT_DECLARE(EgCanBus);
ECS_COMPONENT_DECLARE(EgCanSignal);
ECS_COMPONENT_DECLARE(EgCanBusBook);

ECS_COPY(EgCanBusDescription, dst, src, {
	ecs_os_strset((char **)&dst->interface, src->interface);
})

ECS_MOVE(EgCanBusDescription, dst, src, {
	ecs_os_free((char *)dst->interface);
	dst->interface = src->interface;
	src->interface = NULL;
})

ECS_DTOR(EgCanBusDescription, ptr, {
	ecs_os_free((char *)ptr->interface);
})

static int sockaddr_can_from_interface(struct sockaddr_can *addr, int s, char const *interface)
{
	int rc = 0;
	struct ifreq ifr = {0};
	strcpy(ifr.ifr_name, interface);
	rc = ioctl(s, SIOCGIFINDEX, &ifr);
	if (rc < 0) {
		return rc;
	}
	memset(addr, 0, sizeof(struct sockaddr_can));
	addr->can_family = AF_CAN;
	addr->can_ifindex = ifr.ifr_ifindex;
	return rc;
}

static int socket_from_interace(char const *interface)
{
	printf("CAN Sockets Demo\r\n");
	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("socket()");
		return 1;
	}

	struct sockaddr_can addr = {0};
	int rc = sockaddr_can_from_interface(&addr, s, interface);

	if (rc < 0) {
		return rc;
	}

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		perror("bind()");
		return 1;
	}
	return s;
}

static void System_EgCanBusDescription1(ecs_iter_t *it)
{
	EgCanBusDescription *d = ecs_field(it, EgCanBusDescription, 1);
	for (int i = 0; i < it->count; ++i, ++d) {
		d->signals_amount = 0;
	}
}

static void System_EgCanBusDescription2(ecs_iter_t *it)
{
	EgCanBusDescription *d = ecs_field(it, EgCanBusDescription, 1);
	// EgCanSignal *s = ecs_field(it, EgCanSignal, 2);
	for (int i = 0; i < it->count; ++i) {
		d->signals_amount++;
	}
}

typedef struct {
	int fd_epoll;
	ecs_os_mutex_t lock;
} thread_stuff_t;

static void CanBusDescription_System(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	thread_stuff_t *stuff = it->ctx;
	EgCanBusDescription *d = ecs_field(it, EgCanBusDescription, 1);

	for (int i = 0; i < it->count; ++i, ++d) {
		ecs_entity_t e = it->entities[i];

		EgCanBus *b = ecs_ensure(world, e, EgCanBus);
		b->socket = socket_from_interace(d->interface);
		if (b->socket < 0) {
			continue;
		}

		EgCanBusBook *book = ecs_ensure(world, e, EgCanBusBook);
		book->socket = b->socket;
		book->tx = ecs_os_calloc_n(eg_can_book8_t, EG_CAN_BOOK_CAP);
		book->rx = ecs_os_calloc_n(eg_can_book8_t, EG_CAN_BOOK_CAP);
		book->cap = EG_CAN_BOOK_CAP;

		// https://stackoverflow.com/questions/7058737/is-epoll-thread-safe
		struct epoll_event event_setup = {
		.events = EPOLLIN,
		.data.ptr = book};
		if (epoll_ctl(stuff->fd_epoll, EPOLL_CTL_ADD, b->socket, &event_setup)) {
			perror("failed to add socket to epoll");
			continue;
		}

		// ecs_os_mutex_lock(stuff->lock);
		// ecs_os_mutex_unlock(stuff->lock);
	}
}

#define EG_CAN_CTRLMSG_LEN CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(3 * sizeof(struct timespec)) + CMSG_SPACE(sizeof(__u32))
/* CAN CC/FD/XL frame union */
typedef union {
	struct can_frame cc;
	struct canfd_frame fd;
	// struct canxl_frame xl;
} cu_t;

int eg_can_recv(int s, eg_can_frame_t *frame)
{
	cu_t cu = {0};
	struct iovec iov[1] = {0};
	struct msghdr msg = {0};
	char ctrlmsg[EG_CAN_CTRLMSG_LEN] = {0};
	struct sockaddr_can addr = {0};

	iov[0].iov_base = &cu;

	msg.msg_name = &addr;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &ctrlmsg;
	/* these settings may be modified by recvmsg() */
	msg.msg_iov[0].iov_len = sizeof(cu_t);
	msg.msg_namelen = sizeof(struct sockaddr_can);
	msg.msg_controllen = EG_CAN_CTRLMSG_LEN;
	msg.msg_flags = 0;
	int nbytes = recvmsg(s, &msg, 0);
	if (nbytes < 0) {
		perror("recvmsg()");
		return nbytes;
	}
	// eg_can_print((cu_t *)msg.msg_iov->iov_base, nbytes);

	frame->can_id = cu.cc.can_id;
	frame->len = cu.cc.len;
	memcpy(frame->data, cu.cc.data, CAN_MAX_DLEN);

	return nbytes;
}

#define MAXSOCK 16
static void *thread_loop(thread_stuff_t *stuff)
{
	struct epoll_event events[MAXSOCK] = {0};
	int timeout_ms = -1;
	int running = 1;

	while (running) {

		// Wait for events:
		int num_events = epoll_wait(stuff->fd_epoll, events, MAXSOCK, timeout_ms);
		if (num_events == -1) {
			if (errno != EINTR) {
				running = 0;
			}
			continue;
		}

		// Check every event:
		for (int i = 0; i < num_events; i++) {
			EgCanBusBook *book = events[i].data.ptr;
			// printf("epoll event: %i\n", book->socket);
			eg_can_frame_t frame = {0};
			eg_can_recv(book->socket, &frame);
			ecs_os_mutex_lock(stuff->lock);
			eg_can_book8_t *rx = book->rx + frame.can_id;
			rx->dirty = 1;
			rx->len = frame.len;
			rx->packet[0] = frame.data[0];
			rx->packet[1] = frame.data[1];
			rx->packet[2] = frame.data[2];
			rx->packet[3] = frame.data[3];
			rx->packet[4] = frame.data[4];
			rx->packet[5] = frame.data[5];
			rx->packet[6] = frame.data[6];
			rx->packet[7] = frame.data[7];
			ecs_os_mutex_unlock(stuff->lock);
		}
	}

	return NULL;
}

int eg_can_send(int s, eg_can_frame_t *frame)
{
	assert(sizeof(eg_can_frame_t) == sizeof(struct can_frame));
	int n = write(s, frame, sizeof(eg_can_frame_t));
	if (n != sizeof(eg_can_frame_t)) {
		perror("write()");
		return n;
	}
	return n;
}

static void EgCanBusBook_System_Sender(ecs_iter_t *it)
{
	// ecs_world_t *world = it->world;
	// thread_stuff_t * stuff = it->ctx;
	EgCanBus *bus = ecs_field(it, EgCanBus, 1);
	EgCanBusBook *book = ecs_field(it, EgCanBusBook, 2);
	for (int i = 0; i < it->count; ++i, ++bus, ++book) {
		// ecs_entity_t e = it->entities[i];
		for (int canid = 0; canid < book->cap; ++canid) {
			if (book->tx[canid].dirty) {
				eg_can_frame_t frame = {0};
				frame.can_id = canid;
				frame.len = book->tx[canid].len;
				frame.data[0] = book->tx[canid].packet[0];
				frame.data[1] = book->tx[canid].packet[1];
				frame.data[2] = book->tx[canid].packet[2];
				frame.data[3] = book->tx[canid].packet[3];
				eg_can_send(bus->socket, &frame);
				printf("Send can packet socket=%i\n", bus->socket);
				book->tx[canid].dirty = 0;
			}
		}
	}
}

static void System_Rx(ecs_iter_t *it)
{
	// ecs_world_t *world = it->world;
	thread_stuff_t *stuff = it->ctx;
	EgCanBusBook *book = ecs_field(it, EgCanBusBook, 1);
	EgCanSignal *signal = ecs_field(it, EgCanSignal, 2);
	for (int i = 0; i < it->count; ++i, ++signal) {
		int canid = signal->canid;
		if (canid >= book->cap) {
			continue;
		}
		int byte_offset = signal->byte_offset;
		if (byte_offset >= 8) {
			continue;
		}
		if (byte_offset < 0) {
			continue;
		}

		ecs_os_mutex_lock(stuff->lock);
		int32_t value = book->rx[canid].packet[byte_offset];
		ecs_os_mutex_unlock(stuff->lock);
		signal->rx = value;
	}
}

static void System_Value(ecs_iter_t *it)
{
	EgCanSignal *s = ecs_field(it, EgCanSignal, 1);
	EgQuantitiesProgress *q = ecs_field(it, EgQuantitiesProgress, 2);
	for (int i = 0; i < it->count; ++i, ++s, ++q) {
		q->value = (float)s->rx;
		q->min = (float)s->min;
		q->max = (float)s->max;
	}
}

void EgCanBusBook_prepare_send(EgCanBusBook *book, EgCanSignal *signal)
{
	// printf("Send can packet canid=%i, value=%i\n", (int)signal->canid, signal->value);
	if (signal->canid > EG_CAN_BOOK_CAP) {
		return;
	}
	if (signal->byte_offset >= 8) {
		return;
	}
	if (signal->byte_offset < 0) {
		return;
	}
	uint8_t value = signal->tx;
	book->tx[signal->canid].len = signal->len;
	book->tx[signal->canid].packet[signal->byte_offset] = value;
	book->tx[signal->canid].dirty = 1;
}

typedef struct {
	int clock;
	int bitrate;
} iface_info_t;

void interface_details(char const *iname, iface_info_t * out)
{
	FILE *fp;
	char buf[1035];
	snprintf(buf, sizeof(buf), "ip -details link show %s", iname);
	fp = popen(buf, "r");
	if (fp == NULL) {
		printf("Failed to run command\n");
		exit(1);
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		printf("%s", buf);
		char * p;
		p = strstr(buf, "bitrate");
		if (p != NULL) {
			char * endptr;
			p += sizeof("bitrate");
			intmax_t m = strtoimax(p, &endptr,10);
			out->bitrate = (int)m;
		}
		p = strstr(buf, "clock");
		if (p != NULL) {
			char * endptr;
			p += sizeof("clock");
			intmax_t m = strtoimax(p, &endptr,10);
			out->clock = (int)m;
		}
	}
	pclose(fp);
}

/*
IFF_UP            Interface is running.
IFF_BROADCAST     Valid broadcast address set.
IFF_DEBUG         Internal debugging flag.
IFF_LOOPBACK      Interface is a loopback interface.
IFF_POINTOPOINT   Interface is a point-to-point link.
IFF_RUNNING       Resources allocated.
IFF_NOARP         No arp protocol, L2 destination address not set.
IFF_PROMISC       Interface is in promiscuous mode.
IFF_NOTRAILERS    Avoid use of trailers.
IFF_ALLMULTI      Receive all multicast packets.
IFF_MASTER        Master of a load balancing bundle.
IFF_SLAVE         Slave of a load balancing bundle.
IFF_MULTICAST     Supports multicast
IFF_PORTSEL       Is able to select media type via ifmap.
IFF_AUTOMEDIA     Auto media selection active.
IFF_DYNAMIC       The addresses are lost when the interface goes down.
IFF_LOWER_UP      Driver signals L1 up (since Linux 2.6.17)
IFF_DORMANT       Driver signals dormant (since Linux 2.6.17)
IFF_ECHO          Echo sent packets (since Linux 2.6.25)
*/

void list_interfaces()
{
	struct ifaddrs *ifaddr;
	int n = getifaddrs(&ifaddr);
	int family, s;
	char host[NI_MAXHOST];
	for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (strcmp(ifa->ifa_name, "can0") == 0) {
			printf("%s\n", ifa->ifa_name);
			iface_info_t info;
			interface_details(ifa->ifa_name, &info);
			printf("IFF_UP            : %i\n", !!(ifa->ifa_flags & IFF_UP));
			printf("IFF_BROADCAST     : %i\n", !!(ifa->ifa_flags & IFF_BROADCAST));
			printf("IFF_DEBUG         : %i\n", !!(ifa->ifa_flags & IFF_DEBUG));
			printf("IFF_LOOPBACK      : %i\n", !!(ifa->ifa_flags & IFF_LOOPBACK));
			printf("IFF_POINTOPOINT   : %i\n", !!(ifa->ifa_flags & IFF_POINTOPOINT));
			printf("IFF_RUNNING       : %i\n", !!(ifa->ifa_flags & IFF_RUNNING));
			printf("IFF_NOARP         : %i\n", !!(ifa->ifa_flags & IFF_NOARP));
			printf("IFF_PROMISC       : %i\n", !!(ifa->ifa_flags & IFF_PROMISC));
			printf("IFF_NOTRAILERS    : %i\n", !!(ifa->ifa_flags & IFF_NOTRAILERS));
			printf("IFF_ALLMULTI      : %i\n", !!(ifa->ifa_flags & IFF_ALLMULTI));
			printf("IFF_MASTER        : %i\n", !!(ifa->ifa_flags & IFF_MASTER));
			printf("IFF_SLAVE         : %i\n", !!(ifa->ifa_flags & IFF_SLAVE));
			printf("IFF_MULTICAST     : %i\n", !!(ifa->ifa_flags & IFF_MULTICAST));
			printf("IFF_PORTSEL       : %i\n", !!(ifa->ifa_flags & IFF_PORTSEL));
			printf("IFF_AUTOMEDIA     : %i\n", !!(ifa->ifa_flags & IFF_AUTOMEDIA));
			printf("IFF_DYNAMIC       : %i\n", !!(ifa->ifa_flags & IFF_DYNAMIC));
			// printf("IFF_LOWER_UP      : %i\n", !!(ifa->ifa_flags & IFF_LOWER_UP   ));
			// printf("IFF_DORMANT       : %i\n", !!(ifa->ifa_flags & IFF_DORMANT    ));
			// printf("IFF_ECHO          : %i\n", !!(ifa->ifa_flags & IFF_ECHO       ));
		}
		/*
		if (ifa->ifa_addr == NULL)
		    continue;
		family = ifa->ifa_addr->sa_family;
		printf("%-8s %s (%d)\n",ifa->ifa_name,(family == AF_PACKET) ? "AF_PACKET" :(family == AF_INET) ? "AF_INET":(family == AF_INET6) ? "AF_INET6": "???",family);
		if (family == AF_INET || family == AF_INET6) {
		    s = getnameinfo(ifa->ifa_addr,
		    (family == AF_INET) ? sizeof(struct sockaddr_in) :sizeof(struct sockaddr_in6),host, NI_MAXHOST,
		    NULL, 0, NI_NUMERICHOST);

		    if (s != 0) {
		        printf("getnameinfo() failed: %s\n", gai_strerror(s));
		        exit(EXIT_FAILURE);
		    }
		    printf("\t\taddress: <%s>\n", host);
		} else if (family == AF_PACKET && ifa->ifa_data != NULL) {
		    struct rtnl_link_stats *stats = ifa->ifa_data;
		    printf("\t\ttx_packets = %10u; rx_packets = %10u\n""\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
		    stats->tx_packets, stats->rx_packets,
		    stats->tx_bytes, stats->rx_bytes);
		}
		*/
	}
	freeifaddrs(ifaddr);
}

void EgCanImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgCan);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "EgCan");

	ECS_COMPONENT_DEFINE(world, EgCanEpoll);
	ECS_COMPONENT_DEFINE(world, EgCanBusDescription);
	ECS_COMPONENT_DEFINE(world, EgCanBus);
	ECS_COMPONENT_DEFINE(world, EgCanSignal);
	ECS_COMPONENT_DEFINE(world, EgCanBusBook);

	list_interfaces();

	thread_stuff_t *stuff = ecs_os_calloc_t(thread_stuff_t);
	stuff->lock = ecs_os_mutex_new();
	stuff->fd_epoll = epoll_create(1);
	if (stuff->fd_epoll < 0) {
		perror("epoll_create()");
	} else {
		printf("Starting epoll thread!\n");
		ecs_os_thread_new((ecs_os_thread_callback_t)thread_loop, stuff);
	}

	// clang-format off
	ecs_set_hooks(world, EgCanBusDescription, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgCanBusDescription),
	.copy = ecs_copy(EgCanBusDescription),
	.dtor = ecs_dtor(EgCanBusDescription)
	});

	ecs_struct(world,
	{.entity = ecs_id(EgCanBusDescription),
	.members = {
	{.name = "interface", .type = ecs_id(ecs_string_t)},
	{.name = "signals_amount", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgCanBus),
	.members = {
	{.name = "socket", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(EgCanSignal),
	.members = {
	{.name = "canid", .type = ecs_id(ecs_u32_t)},
	{.name = "len", .type = ecs_id(ecs_i32_t)},
	{.name = "rx", .type = ecs_id(ecs_i32_t)},
	{.name = "tx", .type = ecs_id(ecs_i32_t)},
	{.name = "byte_offset", .type = ecs_id(ecs_i32_t)},
	{.name = "min", .type = ecs_id(ecs_i32_t)},
	{.name = "max", .type = ecs_id(ecs_i32_t)},
	{.name = "gui_index", .type = ecs_id(ecs_i32_t)},
	}});


	ecs_struct(world,
	{.entity = ecs_id(EgCanBusBook),
	.members = {
	{.name = "len", .type = ecs_id(ecs_i32_t)},
	{.name = "book", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_EgCanBusDescription1,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_EgCanBusDescription2,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription), .src.flags = EcsUp, .src.trav = EcsIsA},
	{.id = ecs_id(EgCanSignal)}
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = CanBusDescription_System,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusDescription), .src.flags = EcsSelf},
	{.id = ecs_id(EgCanBus), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = EgCanBusBook_System_Sender,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBus)},
	{.id = ecs_id(EgCanBusBook)},
	}});


	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_Rx,
	.ctx = stuff,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanBusBook), .src.flags = EcsUp, .src.trav = EcsIsA},
	{.id = ecs_id(EgCanSignal)}
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_Value,
	.query.filter.terms =
	{
	{.id = ecs_id(EgCanSignal)},
	{.id = ecs_id(EgQuantitiesProgress)}
	}});
	// clang-format on
}