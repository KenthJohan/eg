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


typedef struct {
	int index;
	int clock;
	int bitrate;
	int mtu;
} iface_info_t;


int eg_can_recv(int s, eg_can_frame_t *frame);
int socket_from_interace(char const *interface);
void interface_details(char const *iname, iface_info_t * out);
void list_interfaces();