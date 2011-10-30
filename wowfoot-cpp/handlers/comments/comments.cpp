#include "comments.h"
#include "commentTab.h"
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
	throw logic_error("sqlite"); } } while(0)

static void closeDb() {
	if(sDB) {
		sqlite3_stmt* stmt = NULL;	//dummy
		SQLT(sqlite3_close(sDB));
	}
}

static string formatComment(const char* src);
static void formatTag(ostream& o, const char* tag, size_t len);
static void formatUrl(ostream& o, const char* url, size_t len);

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

static string formatComment(const char* src) {
	ostringstream o;
	const char* ptr = src;
	while(*ptr) {
		char c = *ptr;
		ptr++;
		if(c == '[') {	// start tag
			const char* endPtr = strchr(ptr, ']');
			if(!endPtr) {
				o << (ptr-1);
				break;
			}
			formatTag(o, ptr, endPtr - ptr);
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

#define COMPLEX_TAG(src, dst) if(strncmp(src, tag, len) == 0) { o << dst; return; }
#define SIMPLE_TAG(t) if(strncmp(t, tag, len) == 0) { o << "<" t ">"; return; }\
if(strncmp("/" t, tag, len) == 0) { o << "</" t ">"; return; }

static void formatTag(ostream& o, const char* tag, size_t len) {
	SIMPLE_TAG("b");
	SIMPLE_TAG("i");
	SIMPLE_TAG("u");
	SIMPLE_TAG("li");
	COMPLEX_TAG("ul", "</p><ul>");
	COMPLEX_TAG("/ul", "</ul><p>");
	COMPLEX_TAG("ol", "</p><ol>");
	COMPLEX_TAG("/ol", "</ol><p>");

	if(strncmp("url=", tag, 4) == 0) {
		const char* url = tag + 4;
		size_t urlLen = len - 4;
		o << "<a href=\"";
		formatUrl(o, url, urlLen);
		o << "\">";
		return;
	}
	COMPLEX_TAG("/url", "</a>");

	// unknown tag
	o << "["<<o.write(tag, len)<<"]";
}

static void formatUrl(ostream& o, const char* url, size_t len) {
	// s/http://*.wowhead.com/
	const char* wh = ".wowhead.com/";
	const char* whf = strstr(url, wh);
	if(whf) {
		const char* path = whf + strlen(wh);
		if(*path == '?')
			path += 1;
		size_t pathLen = len - (path - url);
		o.write(path, pathLen);
		return;
	}

	o.write(url, len);
}
