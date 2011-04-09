#ifndef UTIL_H
#define UTIL_H

bool fileExists(const char* name);

#ifdef WIN32
#include <direct.h>
#else
int mkdir(const char* name);
#endif

#endif	//UTIL_H
