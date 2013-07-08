#include "util.h"
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>

#ifndef WIN32
int mkdir(const char* name) {
	return ::mkdir(name, 0700);
}
#endif
