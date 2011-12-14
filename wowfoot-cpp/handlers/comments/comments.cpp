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
		c.originalBody = (const char*)sqlite3_column_text(stmt, 1);
		for(size_t i=0; i<c.originalBody.size(); i++) {
			if(c.originalBody[i] == '-')
				c.originalBody[i] = '_';
			if(c.originalBody[i] == 'n' && c.originalBody[i-1] == '\\')
				c.originalBody[i] = '\n';
		}
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
		c == '=' || c == '.' || c == ':' || c == '&';
}
static bool isWowheadNonUrlChar(char c) {
	return c == '/' || c == '?' || c == '.';
}

#define TAG_LIST 1
#define TAG_LIST_ITEM 2
#define BETWEEN_LIST (((tagState & TAG_LIST) != 0) && ((tagState & TAG_LIST_ITEM) == 0))

static string formatComment(const char* src) {
	ostringstream o;
	int tagState = 0;
	const char* ptr = src;
	while(*ptr) {
		char c = *ptr;
		if(BETWEEN_LIST && c != '[' && !isspace(c) && c != '\\') {
			o << "<li>";
			tagState |= TAG_LIST_ITEM;
		}
		if(STREQ(ptr, "http://")) {	// unescaped link
			ptr = formatUnescapedUrl(o, ptr);
			continue;
		}
		ptr++;
		if(c == '[') {	// start tag
			while(*ptr == '[') {
				o << '[';
				ptr++;
			}
			const char* endPtr = strchr(ptr, ']');
			if(!endPtr) {
				o << (ptr-1);
				break;
			}
			tagState = formatTag(o, ptr, endPtr - ptr, tagState);
			ptr = endPtr + 1;
		} else if(c == '\\' && *ptr == 'n') {
			ptr++;
			// between [ul] and [li], no text may appear.
			if(!BETWEEN_LIST)
				o << "<br>";
			o << '\n';
		} else if(c == '&') {
			o << "&amp;";
		} else if(c == '<') {
			o << "&lt;";
		} else if(c == '>') {
			o << "&gt;";
		} else {
			o << c;
		}
	}
	return o.str();
}

static bool tagFlag(int& tagState, int flag) {
	if(!flag)
		return true;
	if(!(tagState & flag)) {
		tagState |= flag;
		return true;
	}
	return false;
}

static bool untagFlag(int& tagState, int flag) {
	if(!flag)
		return true;
	if(tagState & flag) {
		tagState &= ~flag;
		return true;
	}
	return false;
}

#define COMPLEX_TAG(src, dst, flag) if(strncmp(src, tag, len) == 0) { o << dst; flag; return tagState; }
#define FLAG_TAG(t, flag, end) \
	if(strncmp(t, tag, len) == 0) if(tagFlag(tagState, flag)) { o << "<" t ">"; return tagState; }\
	if(strncmp("/" t, tag, len) == 0) if(untagFlag(tagState, flag)) { o << "</" t ">"end; return tagState; }
#define SIMPLE_TAG(t) FLAG_TAG(t,0,)

static int formatTag(ostream& o, const char* tag, size_t len, int tagState) {
	//printf("tag: %i %.*s\n", tagState, (int)len, tag);
	SIMPLE_TAG("b");
	SIMPLE_TAG("i");
	COMPLEX_TAG("u", "<span class=\"underlined\">",);
	COMPLEX_TAG("/u", "</span>",);
	if(tagState & TAG_LIST) {
		FLAG_TAG("li", TAG_LIST_ITEM, <<"\n");
	} else {
		// discard invalid tags
		COMPLEX_TAG("li", "\n",);
		COMPLEX_TAG("/li", "\n",);
	}
	COMPLEX_TAG("ul", "</p>\n<ul>", tagState |= TAG_LIST);
	COMPLEX_TAG("/ul", "</ul>\n<p>", tagState &= ~TAG_LIST);
	COMPLEX_TAG("ol", "</p>\n<ol>", tagState |= TAG_LIST);
	COMPLEX_TAG("/ol", "</ol>\n<p>", tagState &= ~TAG_LIST);

	if(strncmp("url=", tag, 4) == 0) {
		//printf("url tag: %i %.*s\n", tagState, (int)len, tag);
		const char* url = tag + 4;
		size_t urlLen = len - 4;
		o << "<a href=\"";
		formatUrl(o, url, urlLen);
		o << "\">";
		return tagState;
	}
	COMPLEX_TAG("/url", "</a>",);

	// unknown tag
	//printf("unknown tag: %i %.*s\n", tagState, (int)len, tag);
	o << "[";
	o.write(tag, len);
	o << "]";
	return tagState;
}

static void streamHtmlEncode(ostream& o, const char* url, size_t len) {
	for(size_t pos = 0; pos < len; pos++) {
		if(url[pos] == '&') {
			o << "&amp;";
		} else if(url[pos] == 0) {
			break;
		} else {
			o << url[pos];
		}
	}
}

static void formatUrl(ostream& o, const char* url, size_t len) {
	// s/http://*.wowhead.com/
	const char* whf = (char*)memmem(url, len, WH, strlen(WH));
	if(whf) {
		const char* path = whf + strlen(WH);
		if(*path == '?')
			path += 1;
		size_t pathLen = len - (path - url);
		streamHtmlEncode(o, path, pathLen);
		return;
	}

	streamHtmlEncode(o, url, len);
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
		streamHtmlEncode(o, path, pathLen);
		o << "\">";
		// todo: write name of linked entity (item, object, spell, quest, et. al)
		streamHtmlEncode(o, path, pathLen);
		o << "</a>";
	} else {
		o << "<a href=\"";
		streamHtmlEncode(o, ptr, urlLen);
		o << "\">";
		streamHtmlEncode(o, ptr, urlLen);
		o << "</a>";
	}
	return end;
}
