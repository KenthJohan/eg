#include "ecsx/ecsx_os.h"
#include <unistd.h>

void ecsx_os_getcwd(char *buf, size_t size)
{
	getcwd(buf, size);
}
