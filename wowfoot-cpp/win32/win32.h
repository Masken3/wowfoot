#ifndef WIN32_H
#define WIN32_H

#include <stdarg.h>

typedef unsigned int uint;

#ifdef WIN32
int asprintf( char **, const char *, ... ) __attribute((format(ms_printf, 2, 3)));
int vasprintf( char **, const char *, va_list );

#ifndef strcasecmp
#define strcasecmp stricmp
#endif
const char* strcasestr(const char* s1, const char* s2);

time_t timegm(struct tm*);

#include "strptime.h"

int utf8towc(wchar_t *p, const char *s, size_t n);

#else
#define O_BINARY 0
#define utf8towc mbtowc
#endif

#endif	//WIN32_H
