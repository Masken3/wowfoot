#ifndef WIN32_H
#define WIN32_H

#include <stdarg.h>

#ifdef WIN32
int asprintf( char **, const char *, ... ) __attribute((format(ms_printf, 2, 3)));
int vasprintf( char **, const char *, va_list );

#define strcasecmp stricmp
const char* strcasestr(const char* s1, const char* s2);
#endif

#endif	//WIN32_H
