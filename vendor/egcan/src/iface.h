#pragma once
#include <stdint.h>

#define EG_CAN_MAX_DLC 8
#define EG_CAN_MAX_RAW_DLC 15
#define EG_CAN_MAX_DLEN 8
#define EG_CAN_MAX_ID 2031 // 0x7EF
typedef struct {
	uint32_t can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
	uint8_t len;      // CAN frame payload length in byte (0 .. EG_CAN_MAX_DLEN)
	uint8_t __pad;    // padding
	uint8_t __res0;   // reserve
	uint8_t len8_dlc; // optional DLC for 8 byte payload length (9 .. 15)
	uint8_t data[EG_CAN_MAX_DLEN] __attribute__((aligned(8)));
} eg_can_frame_t;


/*
1: lo: <LOOPBACK,UP,LOWER_UP> 
mtu 65536 
qdisc noqueue state UNKNOWN mode DEFAULT group default 
qlen 1000
    link/loopback 00:00:00:00:00:00 
	brd 00:00:00:00:00:00 
	promiscuity 0  
	allmulti 0 
	minmtu 0 
	maxmtu 0 
	addrgenmode eui64 
	numtxqueues 1 
	numrxqueues 1 
	gso_max_size 65536 
	gso_max_segs 65535 
	tso_max_size 524280 
	tso_max_segs 65535 
	gro_max_size 65536 
	
2: eno1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP mode DEFAULT group default qlen 1000
    link/ether c8:7f:54:05:d8:dd brd ff:ff:ff:ff:ff:ff promiscuity 0  allmulti 0 minmtu 68 maxmtu 9216 addrgenmode none numtxqueues 4 numrxqueues 4 gso_max_size 65536 gso_max_segs 65535 tso_max_size 65536 tso_max_segs 65535 gro_max_size 65536 parentbus pci parentdev 0000:08:00.0 
    altname enp8s0
3: wlp7s0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT group default qlen 1000
    link/ether 50:c2:e8:d5:87:df brd ff:ff:ff:ff:ff:ff promiscuity 0  allmulti 0 minmtu 256 maxmtu 2304 addrgenmode none numtxqueues 1 numrxqueues 1 gso_max_size 65536 gso_max_segs 65535 tso_max_size 65536 tso_max_segs 65535 gro_max_size 65536 parentbus pci parentdev 0000:07:00.0
*/

typedef struct {
	int index;
	int can_clock; // CAN-bus
	int can_bitrate; // CAN-bus
	int mtu;
	//qdisc noqueue state UNKNOWN mode DEFAULT group default 
	//qlen 1000
    //link/loopback 00:00:00:00:00:00 
	//brd 00:00:00:00:00:00 
	int promiscuity;
	int allmulti;
	int minmtu;
	int maxmtu;
	//addrgenmode eui64 
	int numtxqueues;
	int numrxqueues;
	int gso_max_size;
	int gso_max_segs;
	int tso_max_size;
	int tso_max_segs;
	int gro_max_size;
} iface_info_t;


int eg_can_recv(int s, eg_can_frame_t *frame);
int socket_from_interace(char const *interface);
void interface_details(char const *iname, iface_info_t * out);
void list_interfaces();