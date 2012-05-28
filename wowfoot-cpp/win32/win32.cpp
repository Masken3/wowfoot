#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
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

time_t timegm (struct tm* brokentime) {
	const char* old = getenv("TZ");
	_putenv("TZ=UTC");
	_tzset();
	time_t t = mktime(brokentime);

	// restore old TZ, or remove it if it did not exist.
	char buf[128] = "TZ=";
	if(old)
		strcat(buf, old);
	_putenv(buf);
	_tzset();
	return t;
}


/*
 * A UTF-8 character is converted to a wide character (UTF-32 or UCS-4)
 * using the following rules (binary numbers):
 * \code
 * UTF-32                     - UTF-8
 * 00000000 00000000 0aaaaaaa - 0aaaaaaa
 * 00000000 00000bbb bbaaaaaa - 110bbbbb 10aaaaaa
 * 00000000 ccccbbbb bbaaaaaa - 1110cccc 10bbbbbb 10aaaaaa
 * 000dddcc ccccbbbb bbaaaaaa - 11110ddd 10cccccc 10bbbbbb 10aaaaaa
 * \endcode
 */

typedef unsigned char fuint8_t;
/* Bit mask and bit values of a UTF-8 character lead byte */
static struct { fuint8_t mask; fuint8_t val; } t[4] =
{ { 0x80, 0x00 }, { 0xE0, 0xC0 }, { 0xF0, 0xE0 }, { 0xF8, 0xF0 } };


/** \param lead_byte the first byte of a UTF-8 character;
 *  \retval >0 the length in bytes of the UTF-8 character;
 *  \retval -EILSEQ invalid UTF-8 lead byte;
 *  \remarks For performance reasons, this function does not parse
 *           the whole UTF-8 byte sequence, just the first byte.
 *           If checking the validity of the whole UTF-8 byte sequence
 *           is needed, use #unicode_utf8towc.
 */
int utf8len(int lead_byte);	//unused
int utf8len(int lead_byte)
{
	int k;
	for (k = 0; k < 4; k++)
		if ((lead_byte & t[k].mask) == t[k].val)
			return k + 1;
	return -EILSEQ;
}


/** \param result where to store the converted wide character;
 *  \param string buffer containing the UTF-8 character to convert;
 *  \param size max number of bytes of \c string to examine;
 *  \retval >0 the length in bytes of the processed UTF-8 character, the wide character is stored in \c result;
 *  \retval -EILSEQ invalid UTF-8 byte sequence;
 *  \retval -ENAMETOOLONG \c size too small to parse the UTF-8 character.
 */
int utf8towc(wchar_t* result, const char* string, size_t size)
{
	wchar_t wc = 0;
	unsigned k, j;
	if (!size) return -ENAMETOOLONG;
	for (k = 0; k < 4; k++) {
		if ((*string & t[k].mask) == t[k].val)
		{
			if (size < k + 1) return -ENAMETOOLONG;
			wc = (wchar_t) (unsigned char) *string & ~t[k].mask;
			for (j = 0; j < k; j++)
			{
				if ((*(++string) & 0xC0) != 0x80) return -EILSEQ;
				wc = (wc << 6) | ((wchar_t) (unsigned char) *string & 0x3F);
			}
			*result = wc;
			return k + 1;
		}
	}
	return -EILSEQ;
}
