#pragma once

#include <stdint.h>


int interface_index_from_name(int s, char const *interface);

#define CAN_FRAME_DLEN 8
typedef struct {
	uint32_t can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
	uint8_t len;      // CAN frame payload length in byte (0 .. EG_CAN_MAX_DLEN)
	uint8_t __pad;    // padding
	uint8_t __res0;   // reserve
	uint8_t len8_dlc; // optional DLC for 8 byte payload length (9 .. 15)
	uint8_t data[CAN_FRAME_DLEN] __attribute__((aligned(8)));
} can_frame_t;



int can_socket_from_interface(char const *interface);

int can_send(int fd, can_frame_t *frame);
int can_recv(int fd, can_frame_t *frame);

int can_close(int fd);