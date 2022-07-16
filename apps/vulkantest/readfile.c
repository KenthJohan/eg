#include "readfile.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>





#define ASSERT(e) assert(e)
#define ASSERTF(e,...) assert(e)

char * readfile1(char const * filename, long * l)
{
	//ASSERTF (filename != NULL, "filename is NULL%s", "");
	FILE * file = fopen (filename, "rb");
	//ASSERTF (file != NULL, "can not open file '%s'", filename);
	int r;
	r = fseek (file, 0, SEEK_END);
	//ASSERT (r == 0);
	long length = ftell (file);
	//ASSERT (length >= 0);
	r = fseek (file, 0, SEEK_SET);
	//ASSERT (r == 0);
	char * buffer = (char *) calloc ((unsigned) length + 1, sizeof(char));
	//ASSERTF (buffer != NULL, "buffer is NULL%s", "");
	memset (buffer, 0, (unsigned) length + 1);
	//buffer [length + 1] = 0;
	if (length > 0)
	{
		size_t n = fread (buffer, (unsigned) length, 1, file);
		//ASSERTF (n == 1, "fread error %i %i", (int)n, (int)length);
	}
	fclose (file);
	*l = length;
	return buffer;
}


