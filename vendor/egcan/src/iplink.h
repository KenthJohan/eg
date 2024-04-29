#pragma once


typedef struct {
	char ifname[128];
	int ifindex;
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
} iplink_info_t;

int iplink_parse(iplink_info_t *out, int out_length);