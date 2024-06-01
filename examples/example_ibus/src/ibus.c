#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "ibus.h"

/*
https://github.com/karis79/bmw-ibus/blob/master/bmw-ibus.c
https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
https://github.com/sigrokproject/libserialport/blob/master/serialport.c
*/


#include <stdio.h>
#include <string.h>
#include <assert.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()




/*
 *  supports max 14 channels in this lib (with messagelength of 0x20 there is room for 14 channels)

  Example set of bytes coming over the iBUS line for setting servos: 
    20 40 DB 5 DC 5 54 5 DC 5 E8 3 D0 7 D2 5 E8 3 DC 5 DC 5 DC 5 DC 5 DC 5 DC 5 DA F3
  Explanation
    Protocol length: 20
    Command code: 40 
    Channel 0: DB 5  -> value 0x5DB
    Channel 1: DC 5  -> value 0x5Dc
    Channel 2: 54 5  -> value 0x554
    Channel 3: DC 5  -> value 0x5DC
    Channel 4: E8 3  -> value 0x3E8
    Channel 5: D0 7  -> value 0x7D0
    Channel 6: D2 5  -> value 0x5D2
    Channel 7: E8 3  -> value 0x3E8
    Channel 8: DC 5  -> value 0x5DC
    Channel 9: DC 5  -> value 0x5DC
    Channel 10: DC 5 -> value 0x5DC
    Channel 11: DC 5 -> value 0x5DC
    Channel 12: DC 5 -> value 0x5DC
    Channel 13: DC 5 -> value 0x5DC
    Checksum: DA F3 -> calculated by adding up all previous bytes, total must be FFFF
 */

// packet is <len><cmd><data....><chkl><chkh>, overhead=cmd+chk bytes
// Packets are received very ~7ms so use ~half that for the gap

#define TO16U(x) ((x)[0] | ((x)[1] << 8))



int ibus_open(char const * path)
{
	int rc;
	// Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
	// pi4: /dev/serial0
	// pi5: /dev/ttyAMA0
	//int serial_port = open("/dev/ttyAMA0", O_RDONLY | O_NOCTTY | O_NONBLOCK);
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return fd;
	}

	// Create new termios struct, we call it 'tty' for convention
	struct termios tty;

	// Read in existing settings, and handle any error
	rc = tcgetattr(fd, &tty);
	if (rc != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return rc;
	}

	tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
	tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
	tty.c_cflag |= CS8;            // 8 bits per byte (most common)
	tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;                                                        // Disable echo
	tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
	tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
	tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	// tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	// tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

	tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 32;

	// Set in/out baud rate to be 9600
	rc = cfsetispeed(&tty, B115200);
	if (rc != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return rc;
	}

	rc = cfsetospeed(&tty, B115200);
	if (rc != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return rc;
	}

	// Save tty settings, also checking for error
	rc = tcsetattr(fd, TCSANOW, &tty);
	if (rc != 0) {
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
		return rc;
	}
	return fd;
}


int ibus_close(int fd)
{
	return close(fd);
}


int ibus_parse(char const buf[32], uint16_t ch[10])
{
	if (buf[IBUS_OFFSET_LEN] != 32) {
		return -1;
	}
	if (buf[IBUS_OFFSET_CMD] != IBUS_PROTOCOL_COMMAND40) {
		return -1;
	}
	uint16_t sum = 0xFFFF;
	for(int i = 0; i < 30; ++i) {
		sum -= buf[i];
	}
	uint16_t ssum = TO16U(buf + IBUS_OFFSET_CHK0);
	if (sum != ssum) {
		return IBUS_ERROR_CHKSUM;
	}
	ch[0] = TO16U(buf + IBUS_OFFSET_DATA + 2*0);
	ch[1] = TO16U(buf + IBUS_OFFSET_DATA + 2*1);
	ch[2] = TO16U(buf + IBUS_OFFSET_DATA + 2*2);
	ch[3] = TO16U(buf + IBUS_OFFSET_DATA + 2*3);
	ch[4] = TO16U(buf + IBUS_OFFSET_DATA + 2*4);
	ch[5] = TO16U(buf + IBUS_OFFSET_DATA + 2*5);
	ch[6] = TO16U(buf + IBUS_OFFSET_DATA + 2*6);
	ch[7] = TO16U(buf + IBUS_OFFSET_DATA + 2*7);
	ch[8] = TO16U(buf + IBUS_OFFSET_DATA + 2*8);
	ch[9] = TO16U(buf + IBUS_OFFSET_DATA + 2*9);
	return 0;
}


int ibus_read_sync(int fd, char buf[32])
{
	int n;
	for (int i = 0; i < IBUS_PROTOCOL_LENGTH; ++i) {
		n = read(fd, buf + 0, 1);
		if (n != 1) {
			return -1;
		}
		if (buf[0] != IBUS_PROTOCOL_LENGTH) {
			continue;
		}
		n = read(fd, buf + 1, 1);
		if (n != 1) {
			return -1;
		}
		if (buf[1] != IBUS_PROTOCOL_COMMAND40) {
			continue;
		}
		break;
	}
	n = read(fd, buf + 2, 30);
	if (n != 30) {
		return -1;
	}
	return 0;
}



int ibus_read(int fd, uint16_t ch[IBUS_CHANNEL_COUNT])
{
	char buf[IBUS_PROTOCOL_LENGTH];
	// Normally you wouldn't do this memset() call, but since we will just receive
	// ASCII data for this example, we'll set everything to 0 so we can
	// call printf() easily.
	memset(buf, 0, IBUS_PROTOCOL_LENGTH);

	// Read bytes. The behaviour of read() (e.g. does it block?,
	// how long does it block for?) depends on the configuration
	// settings above, specifically VMIN and VTIME
	int n = read(fd, buf, IBUS_PROTOCOL_LENGTH);
	// n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
	if (n < 0) {
		printf("Error reading: %s", strerror(errno));
		return IBUS_ERROR_READ1;
	}
	if (n != IBUS_PROTOCOL_LENGTH) {
		printf("Error reading not 32 bytes\n");
		return IBUS_ERROR_READ2;
	}

	synced:
	int rc = ibus_parse(buf, ch);
	if (rc == IBUS_ERROR_OFFSET) {
		rc = ibus_read_sync(fd, buf);
		if (rc == 0) {
			printf("synced\n");
			assert(ibus_read_sync(fd, buf) != IBUS_ERROR_OFFSET);
			goto synced;
		}
	} else if (rc == IBUS_ERROR_CHKSUM) {
		printf("IBUS_ERROR_CHKSUM \n");
		return IBUS_ERROR_CHKSUM;
	}

	// Test if sync is working by intruducing offset reading:
	// int n = read(fd, &buf, 1);
	return 0;
}
