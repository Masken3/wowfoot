#include <errno.h>
#include <string.h>
typedef int errno_t;

inline void _get_errno(errno_t* ep) {
	*ep = errno;
}

template<class T> T __max(T a, T b) {
	return a > b ? a : b;
}

#define MAX_PATH 360

inline int _stricmp(const char* a, const char* b) {
	return strcasecmp(a, b);
}

#ifdef WIN32
#define PRuDWORD "lu"
#else
#define PRuDWORD "u"
#endif
