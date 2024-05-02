#pragma once
#include <stdio.h>

/*
  {
    "ifindex": 9,
    "ifname": "can0",
    "flags": [
      "NOARP",
      "UP",
      "LOWER_UP",
      "ECHO"
    ],
    "mtu": 16,
    "qdisc": "pfifo_fast",
    "operstate": "UP",
    "linkmode": "DEFAULT",
    "group": "default",
    "txqlen": 10,
    "link_type": "can",
    "promiscuity": 0,
    "allmulti": 0,
    "min_mtu": 0,
    "max_mtu": 0,
    "linkinfo": {
      "info_kind": "can",
      "info_data": {
        "ctrlmode_supported": [
          "LOOPBACK",
          "LISTEN-ONLY",
          "CC-LEN8-DLC"
        ],
        "state": "ERROR-ACTIVE",
        "restart_ms": 0,
        "bittiming": {
          "bitrate": 500000,
          "sample_point": "0.875",
          "tq": 125,
          "prop_seg": 6,
          "phase_seg1": 7,
          "phase_seg2": 2,
          "sjw": 1,
          "brp": 6
        },
        "bittiming_const": {
          "name": "gs_usb",
          "tseg1": {
            "min": 1,
            "max": 16
          },
          "tseg2": {
            "min": 1,
            "max": 8
          },
          "sjw": {
            "min": 1,
            "max": 4
          },
          "brp": {
            "min": 1,
            "max": 1024
          },
          "brp_inc": 1
        },
        "clock": 48000000
      },
      "info_xstats": {
        "restarts": 0,
        "bus_error": 0,
        "arbitration_lost": 0,
        "error_warning": 4,
        "error_passive": 126,
        "bus_off": 0
      }
    },
    "num_tx_queues": 1,
    "num_rx_queues": 1,
    "gso_max_size": 65536,
    "gso_max_segs": 65535,
    "tso_max_size": 65536,
    "tso_max_segs": 65535,
    "gro_max_size": 65536,
    "parentbus": "usb",
    "parentdev": "1-5:1.0",
    "stats64": {
      "rx": {
        "bytes": 10755,
        "packets": 9761,
        "errors": 0,
        "dropped": 0,
        "over_errors": 0,
        "multicast": 0
      },
      "tx": {
        "bytes": 39,
        "packets": 20,
        "errors": 0,
        "dropped": 0,
        "carrier_errors": 0,
        "collisions": 0
      }
    }
  }
*/
typedef struct {
	char ifname[128];
	char link_type[128];
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
	int min_mtu;
	int max_mtu;
	//addrgenmode eui64 
	int num_tx_queues;
	int num_rx_queues;
	int gso_max_size;
	int gso_max_segs;
	int tso_max_size;
	int tso_max_segs;
	int gro_max_size;
	int stats64_rx_bytes;
	int stats64_rx_packets;
	int stats64_rx_errors;
	int stats64_tx_bytes;
	int stats64_tx_packets;
	int stats64_tx_errors;
} iplink_info_t;

int iplink_parse(iplink_info_t *out, int out_length);

void test_popen(FILE *fp);