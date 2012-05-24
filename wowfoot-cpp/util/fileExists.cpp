#include "fileExists.h"
#include "exception.h"
#include <sys/stat.h>

bool fileExists(const char* filename) {
	struct stat s;
	int res = stat(filename, &s);
	if(res == 0)
		return true;
	if(errno == ENOENT)
		return false;
	throwERRNO();
}
