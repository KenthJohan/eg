#include "wsll.h"

#include <stdlib.h>
#include <assert.h>



// https://github.com/zephyrproject-rtos/zephyr/blob/main/subsys/net/lib/websocket/websocket.c

void wsll_gen_frame_2(uint8_t frame[], int type, uint64_t length)
{
	assert(length <= 125);
	frame[0] = (WS_FIN | type);
	frame[1] = length & 0x7F;
}

void wsll_gen_frame_4(uint8_t frame[], int type, uint64_t length)
{
	assert(length >= 126 && length <= 65535);
	frame[0] = (WS_FIN | type);
	frame[1] = 126;
	frame[2] = (length >> 8) & 255;
	frame[3] = length & 255;
}

void wsll_gen_frame_10(uint8_t frame[], int type, uint64_t length)
{
	assert(length >= 65536);
	frame[0] = (WS_FIN | type);
	frame[1] = 127;
	frame[2] = (unsigned char)((length >> 56) & 255);
	frame[3] = (unsigned char)((length >> 48) & 255);
	frame[4] = (unsigned char)((length >> 40) & 255);
	frame[5] = (unsigned char)((length >> 32) & 255);
	frame[6] = (unsigned char)((length >> 24) & 255);
	frame[7] = (unsigned char)((length >> 16) & 255);
	frame[8] = (unsigned char)((length >> 8) & 255);
	frame[9] = (unsigned char)(length & 255);
}


int is_valid_frame(int opcode)
{
	return (
	opcode == WS_FR_OP_TXT || opcode == WS_FR_OP_BIN ||
	opcode == WS_FR_OP_CONT || opcode == WS_FR_OP_PING ||
	opcode == WS_FR_OP_PONG || opcode == WS_FR_OP_CLSE);
}


int is_control_frame(int frame)
{
	return (frame == WS_FR_OP_CLSE || frame == WS_FR_OP_PING || frame == WS_FR_OP_PONG);
}


int32_t pong_msg_to_int32(uint8_t *msg)
{
	int32_t pong_id;
	/* Decodes as big-endian. */
	pong_id = (msg[3] << 0) | (msg[2] << 8) | (msg[1] << 16) | (msg[0] << 24);
	return (pong_id);
}


void int32_to_ping_msg(int32_t ping_id, uint8_t *msg)
{
	/* Encodes as big-endian. */
	msg[0] = (ping_id >> 24);
	msg[1] = (ping_id >> 16);
	msg[2] = (ping_id >> 8);
	msg[3] = (ping_id >> 0);
}