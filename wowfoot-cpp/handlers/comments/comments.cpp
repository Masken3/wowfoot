#include "comments.h"
#include "commentTab.h"
#include "util/exception.h"
#include <sqlite3.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string.h>

using namespace std;

typedef unsigned char byte;

static sqlite3* sDB;

#define SQLT(func) do { int _res = (func); if(_res != SQLITE_OK) {\
	sqlite3_finalize(stmt); stmt = NULL;\
	printf("%s:%i: %s\n", __FILE__, __LINE__, sqlite3_errmsg(sDB));\
	throw Exception("sqlite"); } } while(0)

static void closeDb() {
	if(sDB) {
		sqlite3_stmt* stmt = NULL;	//dummy
		SQLT(sqlite3_close(sDB));
	}
}

static string formatComment(const char* src);
static int formatTag(ostream& o, const char* tag, size_t len, int tagState);
static void formatUrl(ostream& o, const char* url, size_t len);
static const char* formatUnescapedUrl(ostream& o, const char* ptr);

Tab* getComments(const char* type, int id) {
	sqlite3_stmt* stmt = NULL;
	if(!sDB) {
		SQLT(sqlite3_open("../wowfoot-import/imports.db", &sDB));
		atexit(&closeDb);
	}

	char query[1024];
	sprintf(query, "SELECT user, body, rating, date, indent"
		" FROM comments"
		" INNER JOIN %s_comments on commentId = id"
		" WHERE %s_comments.entry = %i",
		type, type, id);

	SQLT(sqlite3_prepare_v2(sDB, query, -1, &stmt, NULL));

	int res;
	commentTabChtml* ct = new commentTabChtml();
	while((res = sqlite3_step(stmt)) == SQLITE_ROW) {
		Comment c;
		c.user = (const char*)sqlite3_column_text(stmt, 0);
		c.body = formatComment((const char*)sqlite3_column_text(stmt, 1));
		c.rating = sqlite3_column_int(stmt, 2);
		c.date = (const char*)sqlite3_column_text(stmt, 3);
		c.indent = sqlite3_column_int(stmt, 4);
		ct->mComments.push_back(c);
	}
	if(res != SQLITE_DONE) {
		SQLT(res);
	}
	SQLT(sqlite3_finalize(stmt));
	ct->id = "comments";
	ct->title = "Comments";
	ct->count = ct->mComments.size();
	return ct;
}

#define STREQ(src, literal) (strncmp(src, literal, strlen(literal)) == 0)
#define WH ".wowhead.com/"

static bool isUrlChar(char c) {
	//gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
	//sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
	return isalnum(c) || c == '_' || c == '#' || c == '?' || c == '/' || c == '-' ||
		c == '=' || c == '.' || c == ':';
}
static bool isWowheadNonUrlChar(char c) {
	return c == '/' || c == '?' || c == '.';
}

static string formatComment(const char* src) {
	ostringstream o;
	int tagState = 0;
	const char* ptr = src;
	while(*ptr) {
		if(STREQ(ptr, "http://")) {	// unescaped link
			ptr = formatUnescapedUrl(o, ptr);
			continue;
		}
		char c = *ptr;
		ptr++;
		if(c == '[') {	// start tag
			const char* endPtr = strchr(ptr, ']');
			if(!endPtr) {
				o << (ptr-1);
				break;
			}
			tagState = formatTag(o, ptr, endPtr - ptr, tagState);
			ptr = endPtr + 1;
		} else if(c == '\\' && *ptr == 'n') {
			ptr++;
			o << "<br>";
		} else {
			o << c;
		}
	}
	return o.str();
}

#define COMPLEX_TAG(src, dst, flag) if(strncmp(src, tag, len) == 0) { o << dst; flag; return tagState; }
#define SIMPLE_TAG(t) if(strncmp(t, tag, len) == 0) { o << "<" t ">"; return tagState; }\
	if(strncmp("/" t, tag, len) == 0) { o << "</" t ">"; return tagState; }

#define TAG_LIST 1

static int formatTag(ostream& o, const char* tag, size_t len, int tagState) {
	//printf("tag: %i %.*s\n", tagState, len, tag);
	SIMPLE_TAG("b");
	SIMPLE_TAG("i");
	SIMPLE_TAG("u");
	if(tagState & TAG_LIST) {
		SIMPLE_TAG("li");
	} else {
		// discard invalid tags
		COMPLEX_TAG("li", "",);
		COMPLEX_TAG("/li", "",);
	}
	COMPLEX_TAG("ul", "</p><ul>", tagState |= TAG_LIST);
	COMPLEX_TAG("/ul", "</ul><p>", tagState &= ~TAG_LIST);
	COMPLEX_TAG("ol", "</p><ol>", tagState |= TAG_LIST);
	COMPLEX_TAG("/ol", "</ol><p>", tagState &= ~TAG_LIST);

	if(strncmp("url=", tag, 4) == 0) {
		const char* url = tag + 4;
		size_t urlLen = len - 4;
		o << "<a href=\"";
		formatUrl(o, url, urlLen);
		o << "\">";
		return tagState;
	}
	COMPLEX_TAG("/url", "</a>",);

	// unknown tag
	o << "[";
	o.write(tag, len);
	o << "]";
	return tagState;
}

static void formatUrl(ostream& o, const char* url, size_t len) {
	// s/http://*.wowhead.com/
	const char* whf = strstr(url, WH);
	if(whf) {
		const char* path = whf + strlen(WH);
		if(*path == '?')
			path += 1;
		size_t pathLen = len - (path - url);
		o.write(path, pathLen);
		return;
	}

	o.write(url, len);
}

// returns new ptr.
static const char* formatUnescapedUrl(ostream& o, const char* ptr) {
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
	if(isWowhead) {
		o << "<a href=\"";
		o.write(path, pathLen);
		o << "\">";
		// todo: write name of linked entity (item, object, spell, quest, et. al)
		o.write(path, pathLen);
		o << "</a>";
	} else {
		o << "<a href=\"";
		o.write(ptr, urlLen);
		o << "\">";
		o.write(ptr, urlLen);
		o << "</a>";
	}
	return end;
}
