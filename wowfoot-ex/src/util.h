#ifndef UTIL_H
#define UTIL_H

bool fileExists(const char* name);

#ifdef WIN32
#include <direct.h>
#else
int mkdir(const char* name);
#endif

// Avoid "unused variable" warnings in release mode.
#include <assert.h>
#ifdef NDEBUG
#undef assert
#define assert(_Expression)     ((void)0, _Expression)
#endif

#ifdef _MSC_VER
#define PFZT "Iu"
#else
#define PFZT "zu"
#endif

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

template<class T> T MIN(T a, T b) {
	return a < b ? a : b;
}

#endif	//UTIL_H
