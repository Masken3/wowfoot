#include "util.h"
#include <io.h>
#include <fcntl.h>
#include <assert.h>

bool fileExists(const char* name) {
	int fd = open(name, _O_RDWR);
	if(fd < 0)
		return false;
	int res = close(fd);
	assert(res == 0);
	return true;
}

#ifndef WIN32
int mkdir(const char* name) {
	mkdir(name, 0700);
}
#endif
