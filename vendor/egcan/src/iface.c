// #define _GNU_SOURCE /* To get defns of NI_MAXSERV and NI_MAXHOST */
// Fixes: error: variable ‘ifr’ has initializer but incomplete type
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "iface.h"

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
#include <inttypes.h>

static int interface_index_from_name(int s, char const *interface)
{
	int rc = 0;
	struct ifreq ifr = {0};
	strcpy(ifr.ifr_name, interface);
	rc = ioctl(s, SIOCGIFINDEX, &ifr);
	if (rc < 0) {
		return rc;
	}
	return ifr.ifr_ifindex;
}

int socket_from_interace(char const *interface)
{
	printf("CAN Sockets Demo\r\n");
	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("socket()");
		return 1;
	}

	int index = interface_index_from_name(s, interface);
	if (index < 0) {
		return index;
	}

	struct sockaddr_can addr = {0};
	addr.can_family = AF_CAN;
	addr.can_ifindex = index;

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		perror("bind()");
		return 1;
	}
	return s;
}

void interface_details(char const *iname, iface_info_t *out)
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
		//printf("%s", buf);
		char *p;
		p = strstr(buf, "bitrate");
		if (p != NULL) {
			char *endptr;
			p += sizeof("bitrate");
			intmax_t m = strtoimax(p, &endptr, 10);
			out->bitrate = (int)m;
		}
		p = strstr(buf, "clock");
		if (p != NULL) {
			char *endptr;
			p += sizeof("clock");
			intmax_t m = strtoimax(p, &endptr, 10);
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

void print_flags(unsigned int flags)
{
	printf("IFF_UP            : %i\n", !!(flags & IFF_UP));
	printf("IFF_BROADCAST     : %i\n", !!(flags & IFF_BROADCAST));
	printf("IFF_DEBUG         : %i\n", !!(flags & IFF_DEBUG));
	printf("IFF_LOOPBACK      : %i\n", !!(flags & IFF_LOOPBACK));
	printf("IFF_POINTOPOINT   : %i\n", !!(flags & IFF_POINTOPOINT));
	printf("IFF_RUNNING       : %i\n", !!(flags & IFF_RUNNING));
	printf("IFF_NOARP         : %i\n", !!(flags & IFF_NOARP));
	printf("IFF_PROMISC       : %i\n", !!(flags & IFF_PROMISC));
	printf("IFF_NOTRAILERS    : %i\n", !!(flags & IFF_NOTRAILERS));
	printf("IFF_ALLMULTI      : %i\n", !!(flags & IFF_ALLMULTI));
	printf("IFF_MASTER        : %i\n", !!(flags & IFF_MASTER));
	printf("IFF_SLAVE         : %i\n", !!(flags & IFF_SLAVE));
	printf("IFF_MULTICAST     : %i\n", !!(flags & IFF_MULTICAST));
	printf("IFF_PORTSEL       : %i\n", !!(flags & IFF_PORTSEL));
	printf("IFF_AUTOMEDIA     : %i\n", !!(flags & IFF_AUTOMEDIA));
	printf("IFF_DYNAMIC       : %i\n", !!(flags & IFF_DYNAMIC));
	// printf("IFF_LOWER_UP      : %i\n", !!(flags & IFF_LOWER_UP   ));
	// printf("IFF_DORMANT       : %i\n", !!(flags & IFF_DORMANT    ));
	// printf("IFF_ECHO          : %i\n", !!(flags & IFF_ECHO       ));
}

void list_interfaces()
{
	struct ifaddrs *ifaddr;
	int rc = getifaddrs(&ifaddr);
	if (rc < 0) {
		return;
	}
	for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		//if (strcmp(ifa->ifa_name, "can0") == 0) {
			//printf("%s\n", ifa->ifa_name);
			iface_info_t info;
			interface_details(ifa->ifa_name, &info);
			//print_flags(ifa->ifa_flags);
		//}
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