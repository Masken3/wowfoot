#include "util.h"
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>

bool fileExists(const char* name) {
	int fd = open(name, O_RDWR);
	if(fd < 0)
		return false;
	int res = close(fd);
	assert(res == 0);
	return true;
}

#ifndef WIN32
int mkdir(const char* name) {
	return ::mkdir(name, 0700);
}
#endif
