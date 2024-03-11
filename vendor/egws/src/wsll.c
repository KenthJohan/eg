#include "wsll.h"

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>


#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
typedef int socklen_t;
#endif




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
















