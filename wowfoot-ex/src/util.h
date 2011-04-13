#ifndef UTIL_H
#define UTIL_H

bool fileExists(const char* name);

#ifdef WIN32
#include <direct.h>
#else
int mkdir(const char* name);
#endif

template<class T> T MIN(T a, T b) {
	return a < b ? a : b;
}

#endif	//UTIL_H
