#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

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
#define PFZT PRIuPTR
#endif

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

template<class T> T MIN(T a, T b) {
	return a < b ? a : b;
}

template<class T> T MAX(T a, T b) {
	return a > b ? a : b;
}

void writeFile(const char* filename, void* data, size_t size);

#endif	//UTIL_H
