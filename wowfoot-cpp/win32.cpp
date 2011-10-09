#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "win32.h"

int vasprintf( char **sptr, const char *fmt, va_list argv )
{
	int wanted = vsnprintf( *sptr = 0, 0, fmt, argv );
	if( (wanted < 0) || ((*sptr = (char*)malloc( 1 + wanted )) == NULL) )
		return -1;

	return vsprintf( *sptr, fmt, argv );
}

int asprintf( char **sptr, const char *fmt, ... )
{
	int retval;
	va_list argv;
	va_start( argv, fmt );
	retval = vasprintf( sptr, fmt, argv );
	va_end( argv );
	return retval;
}

static const char* strichr(const char* s1, char c) {
	c = toupper(c);
	const char* ptr = s1;
	while(*ptr) {
		if(toupper(*ptr) == c)
			return ptr;
		ptr++;
	}
	return NULL;
}

/*
 * search s2 in s1 -- return pointer to first occurrence of s2
 * or NULL if not found
 */
const char* strcasestr(const char* s1, const char* s2)
{
	const char* c;
	size_t len;

	len = strlen(s2);

	c = strichr(s1, *s2);
	while(c != NULL)
		if(strnicmp(c, s2, len) == 0)
			return(c);
		else {
			c++;
			if(c != NULL)
				c = strichr(c, *s2);
	}
	return NULL;
}
