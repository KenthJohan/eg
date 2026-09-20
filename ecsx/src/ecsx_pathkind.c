#include "ecsx/ecsx_pathkind.h"

#include <string.h>
#include <sys/stat.h>

ecsx_pathkind_t ecsx_pathkind_get_path_type(const char *path)
{
	struct stat path_stat;
	if (strncmp(path, "udp://", 6) == 0) {
		return ECSX_PATHKIND_UDP;
	} else if (strncmp(path, "tcp://", 6) == 0) {
		return ECSX_PATHKIND_TCP;
	} else if (strncmp(path, "http://", 7) == 0) {
		return ECSX_PATHKIND_HTTP;
	}

	if (stat(path, &path_stat) != 0) {
		return ECSX_PATHKIND_NONE;
	}

	if (S_ISREG(path_stat.st_mode)) {
		return ECSX_PATHKIND_FILE;
	} else if (S_ISDIR(path_stat.st_mode)) {
		return ECSX_PATHKIND_DIR;
	} else {
		return ECSX_PATHKIND_OTHER;
	}
}
