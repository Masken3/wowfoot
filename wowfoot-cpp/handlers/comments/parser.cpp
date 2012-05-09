#define __STDC_FORMAT_MACROS
#include "parser.h"

#include "pageTags.h"
#include "util/exception.h"
#include "util/minmax.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#define STREQ(src, literal) (strncmp(src, literal, strlen(literal)) == 0)

#define WH ".wowhead.com/"
#define WW ".wowwiki.com/"

#define FLUSH flush(ptr)

void Parser::flush(const char* end) {
	int len = end - mNodeStart;
	EASSERT(len >= 0);
	if(len > 0) {
		addTextNode(mNodeStart, len);
		mNodeStart = end;
	}
}

void Parser::parse(const char* src) {
	const char* ptr = src;
	mNodeStart = ptr;
	mbtowc(NULL, NULL, 0);	// reset shift state.
	while(*ptr) {
		// skip invalid utf-8 sequences.
		wchar_t w;
		int res = mbtowc(&w, ptr, MB_LEN_MAX);
		if(res <= 0) {
			if(res < 0) {
				printf("Invalid UTF-8 0x%x @ pos %" PRIuPTR "\n", (unsigned char)*ptr, ptr - src);
			}
			FLUSH;
			ptr++;
			mNodeStart = ptr;
			continue;
		} else if(res > 1) {	// valid utf-8 beyond ascii
			ptr += res;
			continue;
		}

		if(STREQ(ptr, "http://")) {	// unescaped link
			FLUSH;
			ptr = parseUnescapedUrl(ptr);
			mNodeStart = ptr;
			continue;
		}

		char c = *ptr;
		ptr++;
		if(c == '[') {	// start tag
			while(*ptr == '[') {
				ptr++;
			}
			const char* endPtr = strchr(ptr, ']');
			if(!endPtr) {
				break;
			}
			flush(ptr-1);
			mNodeStart = ptr;
			parseTag(ptr, endPtr - ptr);
			ptr = endPtr + 1;
			mNodeStart = ptr;
		} else if(c == '\\' && *ptr == 'n') {
			flush(ptr-1);
			ptr++;
			mNodeStart = ptr;
			addLinebreakNode();
		}
	}
	FLUSH;
}

template<class Map>
bool Parser::pageTag(const char* type, size_t typeLen, const char* tag, size_t tagLen,
	Map& map)
{
	if(strncmp(type, tag, typeLen) != 0)
		return false;
	//printf("%*s tag: %.*s\n", (int)(typeLen-1), type, (int)tagLen, tag);
	const char* idString = tag + typeLen;
	size_t idLen = tagLen - typeLen;

	const char* space = (char*)memchr(idString, ' ', idLen);
	if(space)
		idLen = space - idString;

	map.load();
	addPageNode(map, type, idString, idLen);
	return true;
}

enum CompRes {
	crMatch,
	crMismatch,
	crIgnore,
};

static CompRes compareTag(const char* t, size_t tLen, const char* tag, size_t tagLen,
	bool& hasAttributes)
{
	bool match = strncmp(t, tag, tLen) == 0 &&
		(tagLen == tLen || (hasAttributes = isspace(tag[tLen])));
	if(!match)
		return crMismatch;
	return crMatch;
}

// intentional fallthrough
#define COMPARE_TAG(t, matchAction) { CompRes cr = compareTag(t, strlen(t), tag, len, hasAttributes);\
	switch(cr) {\
	case crIgnore: printf("Ignored tag: %s\n", t); return;\
	case crMatch: matchAction\
	case crMismatch: break;\
	} }

#define COMPLEX_TAG(t, type, dst, end) COMPARE_TAG(t, addTagNode(type, tag, len, strlen(t), dst, end); return;)
#define END_TAG(t) COMPARE_TAG(t, addEndTag(t); return;)
#define SIMPLE_TAG(t, type) COMPLEX_TAG(t, type, t, "/" t); END_TAG("/" t)
#define C_TAG(t, type, dst, end) COMPLEX_TAG(t, type, dst, end); END_TAG("/" t)
#define FORMATTING_TAG(t, type) COMPARE_TAG(t, addFormattingTag(tag, len, strlen(t), type); return;); END_TAG("/" t)
#define SPECIAL_TAG(t, addFunc) COMPARE_TAG(t, addFunc; return;); END_TAG("/" t)

void Parser::parseTag(const char* tag, size_t len) {
	bool hasAttributes;
	//printf("tag: %i %.*s\n", tagState, (int)len, tag);
	FORMATTING_TAG("b", BOLD);
	FORMATTING_TAG("i", ITALIC);
	FORMATTING_TAG("small", SMALL);
	FORMATTING_TAG("s", SMALL);
	FORMATTING_TAG("u", UNDERLINED);
	SIMPLE_TAG("table", TABLE);
	SIMPLE_TAG("tr", NO_TYPE);
	SIMPLE_TAG("td", NO_TYPE);
	//SIMPLE_TAG("li", LIST_ITEM);
	SPECIAL_TAG("li", addListItem());
	SPECIAL_TAG("code", addCodeTag());
	SPECIAL_TAG("quote", addQuoteTag());
	SIMPLE_TAG("ul", LIST);
	SIMPLE_TAG("ol", LIST);

	if(!strncmp("url=", tag, 4) || !strncmp("url:", tag, 4) || !strncmp("url ", tag, 4)) {
		//printf("url tag: %i %.*s\n", tagState, (int)len, tag);
		const char* url = tag + 4;
		size_t urlLen = len - 4;
		parseUrl(url, urlLen);
		return;
	}
	C_TAG("url", ANCHOR, "a", "/a");

	if(strncmp("color=", tag, 6) == 0) {
		const char* idString = tag + 6;
		size_t idLen = len - 6;
		addColorTag(idString, idLen);
		return;
	}
	END_TAG("/color");

	// todo: code, quote

#define PAGE_TAG(name, map) if(pageTag(name "=", sizeof(name), tag, len, map)) return;

	PAGE_TAGS(PAGE_TAG);

	// Unknown tag.
	// Assume it is intended to be printed as plain text, with its brackets[].
	printf("unknown tag: %.*s\n", (int)len, tag);
	mNodeStart = tag-1;
	flush(tag+len+1);
}

#ifdef WIN32
#include <algorithm>
static const char* memmem(const char* a, size_t alen, const char* b, size_t blen) {
	const char* res = std::search(a, a+alen, b, b+blen);
	if(res >= a+alen)
		return NULL;
	return res;
}
#endif

static bool isUrlChar(char c) {
	//gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
	//sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
	return isalnum(c) || c == '_' || c == '#' || c == '?' || c == '/' || c == '-' ||
		c == '=' || c == '.' || c == ':' || c == '&' || c == '(' || c == ')' ||
		c == '%';
}
static bool isWowheadNonUrlChar(char c) {
	return c == '/' || c == '?' || c == '.' || c == '-' || c == '&' || ((unsigned int)c) > 127;
}

void Parser::parseUrl(const char* url, size_t len) {
	// skip starting whitespace
	while(isspace(*url)) {
		len--;
		url++;
	}

	// skip broken '['
	if(*url == '[') {
		url++;
		len--;
	}

	// check for start-quote mark.
	if(*url == '"') {
		url++;
		len--;
	}

	// check for whitespace.
	// skip whitespace and any data afterwards.
	const char* ptr = url;
	const char* end = url + len;
	while(ptr < end) {
		// also check for end-quote mark and the invalid start-tag marker.
		if(isspace(*ptr) || *ptr == '"' || *ptr == '[')
			break;
		ptr++;
	}
	len = ptr - url;

	// s/http://*.wowhead.com/
	const char* whf = (char*)memmem(url, len, WH, strlen(WH));
	if(whf) {
		const char* path = whf + strlen(WH);
		if(*path == '?')
			path += 1;
		size_t pathLen = len - (path - url);

		// cut off broken parts
		const char* c = path;
		while(c < (path + pathLen)) {
			if(isWowheadNonUrlChar(*c))
				break;
			c++;
		}
		pathLen = c - path;

		addWowfootUrlNode(path, pathLen);
		return;
	}
	// s/http://*.wowwiki.com/
	//printf("URL test: %*s\n", (int)len, url);
	const char* wwf = (char*)memmem(url, len, WW, strlen(WW));
	if(wwf) {
		const char* path = wwf + strlen(WW);
		size_t pathLen = len - (path - url);
		addWowpediaUrlNode(path, pathLen);
		return;
	}

	// old-style wowhead paths.
	if(strncmp(url, "/?", 2) == 0) {
		url += 2;
		len -= 2;
	}

	addUrlNode(url, len);
}

// returns new ptr.
const char* Parser::parseUnescapedUrl(const char* ptr) {
	const char* domain = ptr + 7;
	const char* slash = strchr(domain, '/');
	if(!slash)
		return domain;
	const char* path = slash + 1;

	// we can assume wowhead urls never have slashes '/',
	// or '?', except as the first character in the path.
	const char* subdomain = slash - (strlen(WH) - 1);
	//printf("sd: %s\n", subdomain);
	bool isWowhead = STREQ(subdomain, WH);
	if(isWowhead && *path == '?')
		path++;
	const char* end = path;
	while(isUrlChar(*end)) {
		if(isWowhead && isWowheadNonUrlChar(*end))
			break;
		end++;
	}
	size_t pathLen = end - path;
	size_t urlLen = end - ptr;
	//printf("uu: %i %.*s\n", isWowhead, urlLen, ptr);
	size_t len;
	if(isWowhead) {
		// write name of linked entity (item, object, spell, quest, et. al)
		// use PAGE_TAG code
#define UE_TAG(name, map) if(pageTag(name "=", sizeof(name), path, pathLen, map)) return end;
		PAGE_TAGS(UE_TAG);

		len = pathLen;
	} else {
		len = urlLen;
	}

	// wowwiki->wowpedia
	const char* wwf = (char*)memmem(ptr, urlLen, WW, strlen(WW));
	if(wwf) {
		addWowpediaUrlNode(path, pathLen);
		addStaticTextNode("http://www.wowpedia.org/");
		addTextNode(path, pathLen);
		addUrlEndNode();
		return end;
	}

	if(!isWowhead)
		path = ptr;

	addUrlNode(path, len);
	addTextNode(path, len);
	addUrlEndNode();
	return end;
}
