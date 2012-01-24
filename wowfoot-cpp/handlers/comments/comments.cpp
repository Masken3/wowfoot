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

struct Tag {
	const char* t;
	size_t len;
};
typedef stack<Tag> TagStack;

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
static int formatTag(ostream& o, const char* tag, size_t len, int tagState, TagStack&);
static void formatUrl(ostream& o, const char* url, size_t len);
static const char* formatUnescapedUrl(ostream& o, const char* ptr);

static bool compareTag(const char* src, size_t srcLen, const char* tag, size_t tagLen,
	bool& hasAttributes, TagStack& tagStack)
{
	bool match = strncmp(src, tag, srcLen) == 0 &&
		(tagLen == srcLen || (hasAttributes = isspace(tag[tagLen])));
	if(!match)
		return false;
	if(src[0] == '/') {	// end tag
		bool stackMatch = false;
		if(!tagStack.empty()) {
			const Tag& t(tagStack.top());
			stackMatch = t.len == srcLen-1 && strncmp(src+1, t.t, t.len) == 0;
		}
		if(stackMatch) {
			tagStack.pop();
		} else {
			printf("Superflous end tag detected: %.*s\n", (int)tagLen, tag);
		}
		return stackMatch;
	} else {	// start tag
		Tag t = { src, srcLen };
		tagStack.push(t);
		return true;
	}
}

#define PUSH(literal) { Tag t = { literal, sizeof(literal)-1 }; tagStack.push(t); }

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
#define TAG_ANCHOR 4
#define TAG_TABLE 8
#define BETWEEN_LIST (((tagState & TAG_LIST) != 0) && ((tagState & TAG_LIST_ITEM) == 0))
#define ALLOW_BR (!BETWEEN_LIST && !(tagState & TAG_TABLE))
#define IN_ANCHOR ((tagState & TAG_ANCHOR) != 0)
#define IN_LIST_ITEM ((tagState & (TAG_LIST|TAG_LIST_ITEM)) == (TAG_LIST|TAG_LIST_ITEM))

static string formatComment(const char* src) {
	ostringstream o;
	int tagState = 0;
	TagStack ts;
	const char* ptr = src;
	while(*ptr) {
		char c = *ptr;
		if(BETWEEN_LIST && c != '[' && !isspace(c) && c != '\\') {
			o << "<li>";
			tagState |= TAG_LIST_ITEM;
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
			tagState = formatTag(o, ptr, endPtr - ptr, tagState, ts);
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
				printf("backslash\n");
				return false;
			}
		}
		if(!isspace(*ptr))
			break;
		ptr++;
	}
	if(*ptr != '[') {
		printf("not a tag\n");
		return false;
	}
	ptr++;
	while(*tags) {
		if(STREQ(ptr, *tags))
			return true;
		tags++;
	}
	printf("not a matching tag\n");
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

#define COMPARE_TAG(t) (compareTag(t, strlen(t), tag, len, hasAttributes, tagStack))

#define STREAM_TAG_ATTRS(t) ; if(hasAttributes) streamTagAttrs(o, tag + strlen(t)); o <<

#define COMPLEX_TAG(src, dst, flag) if COMPARE_TAG(src) { o << dst; flag; return tagState; }
#define FLAG_TAG(t, flag, end) \
	if COMPARE_TAG(t) if(tagFlag(tagState, flag)) { o << "<" t STREAM_TAG_ATTRS(t) ">"; return tagState; }\
	if COMPARE_TAG("/" t) if(untagFlag(tagState, flag)) { o << "</" t ">"end; return tagState; }
#define SIMPLE_TAG(t) CHECK_TAG(t,"<"t STREAM_TAG_ATTRS(t) ">",); COMPLEX_TAG("/" t, "</"t">",)
#define CHECK_TAG(t, dst, flag) \
	if(COMPARE_TAG(t) && nextEndTagIs(tag+len, "/" t)) { o << dst; flag; return tagState; }

static int formatTag(ostream& o, const char* tag, size_t len, int tagState, TagStack& tagStack) {
	bool hasAttributes;
	//printf("tag: %i %.*s\n", tagState, (int)len, tag);
	SIMPLE_TAG("b");
	SIMPLE_TAG("i");
	CHECK_TAG("small", "<span class=\"small\">",);
	COMPLEX_TAG("/small", "</span>",);
	FLAG_TAG("table", TAG_TABLE,);
	SIMPLE_TAG("tr");
	SIMPLE_TAG("td");
	CHECK_TAG("u", "<span class=\"underlined\">",);
	COMPLEX_TAG("/u", "</span>",);
	if(tagState & TAG_LIST) {
		if(strncmp("li", tag, len) == 0) {
			if(IN_LIST_ITEM) {
				o << "</li>";
				tagState &= ~TAG_LIST_ITEM;
			}
			if(tagFlag(tagState, TAG_LIST_ITEM)) {
				o << "<li>";
				return tagState;
			}
		}
		if(strncmp("/li", tag, len) == 0) {
			// if any of these tags follow /li, print /li.
			// otherwise, discard it to maintain valid HTML.
			static const char* sEndLiTags[] = { "/ul]", "/ol]", "li]", NULL };
			if(untagFlag(tagState, TAG_LIST_ITEM)) {
				if(tagsFollow(tag + len+1, sEndLiTags)) {
					o << "</li>";
				} else {
					o << "</li><li>";
					tagState |= TAG_LIST_ITEM;
				}
				return tagState;
			}
		}
	} else {
		// discard invalid tags
		COMPLEX_TAG("li", "\n",);
		COMPLEX_TAG("/li", "\n",);
		tagState &= ~TAG_LIST_ITEM;
	}
	if(tagState & TAG_TABLE) {
		FLAG_TAG("ul", TAG_LIST,);
		FLAG_TAG("ol", TAG_LIST,);
	} else {
		COMPLEX_TAG("ul", "</p>\n<ul>", tagState |= TAG_LIST);
		COMPLEX_TAG("/ul", "</ul>\n<p>", tagState &= ~TAG_LIST);
		COMPLEX_TAG("ol", "</p>\n<ol>", tagState |= TAG_LIST);
		COMPLEX_TAG("/ol", "</ol>\n<p>", tagState &= ~TAG_LIST);
	}

	if(strncmp("url=", tag, 4) == 0) {
		//printf("url tag: %i %.*s\n", tagState, (int)len, tag);
		const char* url = tag + 4;
		size_t urlLen = len - 4;
		o << "<a href=\"";
		formatUrl(o, url, urlLen);
		o << "\">";
		PUSH("url");
		return tagState | TAG_ANCHOR;
	}
	COMPLEX_TAG("/url", "</a>", tagState &= ~TAG_ANCHOR);

	if(strncmp("color=", tag, 6) == 0) {
		const char* idString = tag + 6;
		size_t idLen = len - 6;
		o << "<span class=\"";
		o.write(idString, idLen);
		o << "\">";
		PUSH("color");
		return tagState;
	}
	COMPLEX_TAG("/color", "</span>",);

#define PAGE_TAG(name, map, type) if(pageTag(name "=", sizeof(name), tag, len, map, o)) return tagState;

	PAGE_TAG("spell", gSpells, Spell);
	PAGE_TAG("item", gItems, Item);
	PAGE_TAG("faction", gFactions, Faction);
	PAGE_TAG("npc", gNpcs, Npc);
	PAGE_TAG("achievement", gAchievements, Achievement);
	PAGE_TAG("zone", gWorldMapAreas, WorldMapArea);
	PAGE_TAG("object", gObjects, Object);
	PAGE_TAG("quest", gQuests, Quest);

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
	if(isWowhead) {
		o << "<a href=\"";
		streamHtmlEncode(o, path, pathLen);
		o << "\">";
		// todo: write name of linked entity (item, object, spell, quest, et. al)
		// use PAGE_TAG code
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
