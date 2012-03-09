#include "comments.h"
#include "dbcSpell.h"
#include "dbcAchievement.h"
#include "dbcFaction.h"
#include "dbcWorldMapArea.h"
#include "db_item.h"
#include "db_gameobject_template.h"
#include "db_quest.h"
#include "db_creature_template.h"
#include "commentTab.h"
#include "pageTags.h"
#include "util/exception.h"
#include "util/minmax.h"
#include <sqlite3.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string.h>
#include <stack>

using namespace std;

typedef unsigned char byte;

#define TAG_TYPES(m) \
	m(NO_TYPE, true)\
	m(LIST, true)\
	m(LIST_ITEM, true)\
	m(TABLE, true)\
	m(ANCHOR, false)\
	m(BOLD, false)\
	m(ITALIC, false)\
	m(UNDERLINE, true)\
	m(SMALL, true)\
	m(COLOR, true)\

enum TagType {
#define _TAG_TYPE_ENUM(name, allowMultiple) name,
	TAG_TYPES(_TAG_TYPE_ENUM)
	_TAG_TYPE_COUNT
};

static const bool sTagTypeAllowMultiple[] = {
#define _TAG_TYPE_ALLOW(name, allowMultiple) allowMultiple,
	TAG_TYPES(_TAG_TYPE_ALLOW)
};

struct Tag {
	const char* t;
	size_t len;
	TagType type;
};
class TagStack : public stack<Tag> {
public:
#if 0
	bool contains(const Tag& t) const {
		container_type::const_iterator itr = c.begin();
		for(; itr != c.end(); ++itr) {
			if(itr->len == t.len && strcmp(itr->t, t.t) == 0)
				return true;
		}
		return false;
	}
#endif
	void dump() {
		container_type::const_iterator itr = c.begin();
		for(; itr != c.end(); ++itr) {
			printf("[%.*s]", (int)itr->len, itr->t);
		}
		printf("\n");
	}
};

struct TagState {
	TagStack ts;
	// counts of these tag types on the stack.
	int count[_TAG_TYPE_COUNT];
};

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
static void formatTag(ostream& o, const char* tag, size_t len, TagState&);
static void formatUrl(ostream& o, const char* url, size_t len);
static const char* formatUnescapedUrl(ostream& o, const char* ptr);
static bool tagsFollow(const char* ptr, const char** tags);

static void closeUnclosedTag(ostream& o, TagState& ts) {
	Tag t = ts.ts.top();
	ts.count[t.type]--;
	ts.ts.pop();
	printf("Closing unclosed tag: %*s\n", (int)t.len, t.t);
	o << "</";
	o.write(t.t, t.len);
	o << ">";
}

enum CompRes {
	crMatch,
	crMismatch,
	crIgnore,
};

static CompRes compareTag(ostream& o, const char* src, size_t srcLen, const char* tag, size_t tagLen,
	bool& hasAttributes, TagState& ts, TagType type)
{
	bool match = strncmp(src, tag, srcLen) == 0 &&
		(tagLen == srcLen || (hasAttributes = isspace(tag[tagLen])));
	if(!match)
		return crMismatch;
	if(src[0] == '/') {	// end tag
		CompRes cr = crIgnore;
		if(!ts.ts.empty()) while(ts.count[type] > 0 && cr == crIgnore) {
			const Tag& t(ts.ts.top());
			if(t.len == srcLen-1 && strncmp(src+1, t.t, t.len) == 0)
				cr = crMatch;
			else {
				printf("tag stack top not matching [%.*s]: is [%.*s]\n",
					(int)tagLen, tag, (int)t.len, t.t);
				ts.ts.dump();
				closeUnclosedTag(o, ts);
			}
		}
		if(cr == crMatch) {
			ts.count[type]--;
			ts.ts.pop();
		} else {
			// this message is now potentially false
			//printf("Superflous end tag detected: %.*s\n", (int)tagLen, tag);
		}
		return cr;
	} else {	// start tag
		// check that tag isn't already on the stack
		if(ts.count[type] > 0 && !sTagTypeAllowMultiple[type])
			return crIgnore;

		Tag t = { src, srcLen, type };

		// also check that it isn't immediately followed by its end tag.
		if(tag[tagLen] == ']' && tag[tagLen+1] == '[' &&
			tag[tagLen+2] == '/' && tag[tagLen+3+tagLen] == ']' &&
			strncmp(tag + tagLen + 3, tag, tagLen) == 0)
			return crIgnore;

		// push tag onto stack
		ts.count[type]++;
		ts.ts.push(t);
		return crMatch;
	}
}

#define PUSH(literal, type) { Tag t = { literal, sizeof(literal)-1, type }; ts.ts.push(t); ts.count[type]++; }

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
#define WW ".wowwiki.com/"

static bool isUrlChar(char c) {
	//gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
	//sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
	return isalnum(c) || c == '_' || c == '#' || c == '?' || c == '/' || c == '-' ||
		c == '=' || c == '.' || c == ':' || c == '&';
}
static bool isWowheadNonUrlChar(char c) {
	return c == '/' || c == '?' || c == '.';
}

#define BETWEEN_LIST ((ts.count[LIST] > 0) && (ts.count[LIST_ITEM] < ts.count[LIST]))
#define ALLOW_BR (!BETWEEN_LIST && ts.count[TABLE] == 0 && !pNeeded)
#define IN_ANCHOR (ts.count[ANCHOR] != 0)
#define IN_LIST_ITEM (ts.count[LIST_ITEM] == ts.count[LIST] && ts.count[LIST] > 0)

static string formatComment(const char* src) {
	ostringstream o;
	TagState ts;
	memset(ts.count, 0, sizeof(ts.count));
	const char* ptr = src;
	bool pNeeded = true;
	while(*ptr) {
		char c = *ptr;
		if(pNeeded && ts.ts.empty() && !isspace(c) && c != '\\') {
			static const char* liTags[] = { "ul]", "ol]" };
			if(!tagsFollow(ptr, liTags)) {
				o << "<p>\n";
				pNeeded = false;
			}
		}
		if(ts.count[LIST] > 0) {
			pNeeded = true;
		}
		if(BETWEEN_LIST && !isspace(c) && c != '\\') {
			if(!(c == '[' && (ptr[1] == '/' || STREQ(ptr+1, "li]")))) {
				o << "<li>";
				PUSH("li", LIST_ITEM);
				//printf("inserted <li>. ");
				//ts.ts.dump();
			}
		}
		if(STREQ(ptr, "http://") && !IN_ANCHOR) {	// unescaped link
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
			formatTag(o, ptr, endPtr - ptr, ts);
			ptr = endPtr + 1;
		} else if(c == '\\' && *ptr == 'n') {
			ptr++;
			// between [ul] and [li], only whitespace may appear.
			if(ALLOW_BR)
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
	while(!ts.ts.empty()) {
		closeUnclosedTag(o, ts);
	}
	return o.str();
}

// returns true if any of \a tags follow \a ptr.
// \a tags is a NULL-terminated array of strings.
static bool tagsFollow(const char* ptr, const char** tags) {
	//printf("tagsFollow %s\n", ptr);
	// skip whitespace
	while(*ptr) {
		if(*ptr == '\\') {
			ptr++;
			if(*ptr == 'n') {
				ptr++;
				continue;
			} else {
				//printf("backslash\n");
				return false;
			}
		}
		if(!isspace(*ptr))
			break;
		ptr++;
	}
	if(*ptr != '[') {
		//printf("not a tag\n");
		return false;
	}
	ptr++;
	while(*tags) {
		if(STREQ(ptr, *tags))
			return true;
		tags++;
	}
	//printf("not a matching tag\n");
	return false;
}

static bool nextEndTagIs(const char* ptr, const char* tag) {
	const char* k = strchr(ptr, '[');
	if(!k)
		return false;
	const char* e = strchr(k, ']');
	if(!e)
		return false;
	if(k[1] != '/')
		return true;
	return strncmp(k+1, tag, (e-k)-1) == 0;
}

// attrs ends with ']'
static void streamTagAttrs(ostream& o, const char* attrs) {
	const char* p = attrs;
	while(*p != ']') {
		// skip whitespace
		while(isspace(*p))
			p++;

		// stream name
		o << ' ';
		while(*p != '=') {
			o << *p;
			p++;
		}
		p++;
		o << "=\"";
		while(*p != ']' && !isspace(*p)) {
			o << *p;
			p++;
		}
		o << "\"";
	}
}

template<class T> void streamName(ostream& o, const T& t) {
	o << t.name;
}
template<> void streamName<Quest>(ostream& o, const Quest& t) {
	o << t.title;
}

template<class Map>
static bool pageTag(const char* type, size_t typeLen, const char* tag, size_t tagLen,
	Map& map, ostream& o)
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
	o << "<a href=\""<<type;
	o.write(idString, idLen);
	o << "\">";
	char* end;
	errno = 0;
	int id = strtol(idString, &end, 10);
	const auto* s = map.find(id);
	if(id < 0 || errno != 0 || end != idString + idLen || !s) {
		// error parsing or invalid id.
		o << "["<<type;
		o.write(idString, idLen);
		o << "]";
	} else {
		streamName(o, *s);
	}
	o << "</a>";
	return true;
}

// intentional fallthrough
#define COMPARE_TAG(t, type, matchAction) { CompRes cr = compareTag(o, t, strlen(t), tag, len, hasAttributes, ts, type);\
	switch(cr) {\
	case crIgnore: printf("Ignored tag: %s\n", t); return;\
	case crMatch: matchAction\
	case crMismatch: break;\
	} }

#define STREAM_TAG_ATTRS(t) ; if(hasAttributes) streamTagAttrs(o, tag + strlen(t)); o <<

#define COMPLEX_TAG(src, type, dst) COMPARE_TAG(src, type, o << dst; return;)
// differs from simple_tag in that it allows other tags inside?
#define FLAG_TAG(t, type) \
	COMPARE_TAG(t, type, o << "<" t STREAM_TAG_ATTRS(t) ">"; return;)\
	COMPARE_TAG("/" t, type, o << "</" t ">"; return;)
#define SIMPLE_TAG(t, type) CHECK_TAG(t, type,"<"t STREAM_TAG_ATTRS(t) ">"); COMPLEX_TAG("/" t, type, "</"t">")
#define CHECK_TAG(t, type, dst) \
	COMPARE_TAG(t, type, if(nextEndTagIs(tag+len, "/" t)) { o << dst; return; } else { ts.ts.pop(); ts.count[type]--; } )

static void formatTag(ostream& o, const char* tag, size_t len, TagState& ts) {
	bool hasAttributes;
	//printf("tag: %i %.*s\n", tagState, (int)len, tag);
	SIMPLE_TAG("b", BOLD);
	SIMPLE_TAG("i", ITALIC);
	CHECK_TAG("small", SMALL, "<span class=\"small\">");
	COMPLEX_TAG("/small", SMALL, "</span>");
	FLAG_TAG("table", TABLE);
	SIMPLE_TAG("tr", NO_TYPE);
	SIMPLE_TAG("td", NO_TYPE);
	CHECK_TAG("u", UNDERLINE, "<span class=\"underlined\">");
	COMPLEX_TAG("/u", UNDERLINE, "</span>");
	if(ts.count[LIST] > 0) {
		if(IN_LIST_ITEM) {
			COMPLEX_TAG("li", NO_TYPE, "</li><li>"; ts.ts.pop(); printf("combo </li> "); ts.ts.dump(););
		}
		SIMPLE_TAG("li", LIST_ITEM);
	} else {
		// discard invalid tags
		COMPLEX_TAG("li", NO_TYPE, "\n"; ts.ts.pop());
		COMPLEX_TAG("/li", NO_TYPE, "\n"; ts.ts.pop());
	}
	if(ts.count[TABLE] > 0 || ts.count[LIST] > 0) {
		SIMPLE_TAG("ul", LIST);
		SIMPLE_TAG("ol", LIST);
	} else {
#define ENDP (ts.count[LIST] == 0 ? "</p>\n" : "") <<
		COMPLEX_TAG("ul", LIST, ENDP "<ul>");
		COMPLEX_TAG("/ul", LIST, "</ul>\n");
		COMPLEX_TAG("ol", LIST, ENDP "<ol>");
		COMPLEX_TAG("/ol", LIST, "</ol>\n");
	}

	if(!strncmp("url=", tag, 4) || !strncmp("url:", tag, 4)) {
		//printf("url tag: %i %.*s\n", tagState, (int)len, tag);
		const char* url = tag + 4;
		size_t urlLen = len - 4;
		o << "<a href=\"";
		formatUrl(o, url, urlLen);
		o << "\">";
		PUSH("url", ANCHOR);
		return;
	}
	COMPLEX_TAG("/url", ANCHOR, "</a>");

	if(strncmp("color=", tag, 6) == 0) {
		const char* idString = tag + 6;
		size_t idLen = len - 6;
		o << "<span class=\"";
		o.write(idString, idLen);
		o << "\">";
		PUSH("color", COLOR);
		return;
	}
	COMPLEX_TAG("/color", COLOR, "</span>");

#define PAGE_TAG(name, map) if(pageTag(name "=", sizeof(name), tag, len, map, o)) return;

	PAGE_TAGS(PAGE_TAG);

	// unknown tag
	//printf("unknown tag: %i %.*s\n", tagState, (int)len, tag);
	o << "[";
	o.write(tag, len);
	o << "]";
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

#ifdef WIN32
#include <algorithm>
static const char* memmem(const char* a, size_t alen, const char* b, size_t blen) {
	const char* res = std::search(a, a+alen, b, b+blen);
	if(res >= a+alen)
		return NULL;
	return res;
}
#endif

static void formatUrl(ostream& o, const char* url, size_t len) {
	// check for whitespace.
	// skip whitespace and any data afterwards.
	const char* ptr = url;
	const char* end = url + len;
	while(ptr < end) {
		if(isspace(*ptr))
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
		streamHtmlEncode(o, path, pathLen);
		return;
	}
	// s/http://*.wowwiki.com/
	//printf("URL test: %*s\n", (int)len, url);
	const char* wwf = (char*)memmem(url, len, WW, strlen(WW));
	if(wwf) {
		const char* path = wwf + strlen(WW);
		size_t pathLen = len - (path - url);
		o << "http://www.wowpedia.org/";
		streamHtmlEncode(o, path, pathLen);
		return;
	}
	if(strncmp(url, "/?", 2) == 0) {
		url += 2;
		len -= 2;
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
	size_t len;
	if(isWowhead) {
		// write name of linked entity (item, object, spell, quest, et. al)
		// use PAGE_TAG code
#define UE_TAG(name, map) if(pageTag(name "=", sizeof(name), path, pathLen, map, o)) return end;
		PAGE_TAGS(UE_TAG);

		len = pathLen;
	} else {
		len = urlLen;
	}

	// wowwiki->wowpedia
	const char* wwf = (char*)memmem(ptr, urlLen, WW, strlen(WW));
	if(wwf) {
		o << "<a href=\"http://www.wowpedia.org/";
		streamHtmlEncode(o, path, pathLen);
		o << "\">http://www.wowpedia.org/";
		streamHtmlEncode(o, path, pathLen);
		o << "</a>";
		return end;
	}

	if(!isWowhead)
		path = ptr;

	o << "<a href=\"";
	streamHtmlEncode(o, path, len);
	o << "\">";
	streamHtmlEncode(o, path, len);
	o << "</a>";
	return end;
}
