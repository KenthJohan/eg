#include "ibus.h"

#include <stdio.h>

int main()
{
	int fd = ibus_open("/dev/ttyAMA0");
	if (fd < 0) {
		return fd;
	}

	uint16_t ch[10];
	while (1) {
		int rc = ibus_read(fd, ch);
		printf("%04i: ", rc);
		printf("%04i ", ch[0]);
		printf("%04i ", ch[1]);
		printf("%04i ", ch[2]);
		printf("%04i ", ch[3]);
		printf("%04i ", ch[4]);
		printf("%04i ", ch[5]);
		printf("%04i ", ch[6]);
		printf("%04i ", ch[7]);
		printf("%04i ", ch[8]);
		printf("%04i ", ch[9]);
		printf("\n");
		
	}

	ibus_close(fd);
	return 0;
};


