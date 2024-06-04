#pragma once
#include <stdint.h>

#define IBUS_OFFSET_LEN 0
#define IBUS_OFFSET_CMD 1
#define IBUS_OFFSET_DATA 2
#define IBUS_OFFSET_CHK0 30
#define IBUS_OFFSET_CHK1 31

#define IBUS_ERROR_READ1 -1
#define IBUS_ERROR_READ2 -2
#define IBUS_ERROR_OFFSET -3
#define IBUS_ERROR_CHKSUM -4

#define IBUS_PROTOCOL_COMMAND40 0x40
#define IBUS_PROTOCOL_LENGTH 32
#define IBUS_CHANNEL_COUNT 10


int ibus_open(char const * path);
int ibus_close(int fd);
int ibus_parse(char const buf[32], uint16_t ch[10]);
int ibus_read_sync(int fd, char buf[32]);
int ibus_read(int fd, uint16_t ch[IBUS_CHANNEL_COUNT]);