#include "formatter.h"
#include "node.h"
#include "pageTags.h"
#include "util/exception.h"
#include "util/minmax.h"
#include <sstream>

using namespace std;

string Formatter::formatComment(const char* src) {
	Formatter f;
	f.parse(src);
	//f.compile();
	f.optimize();
	return f.print();
}

Formatter::Formatter() {
}

void Formatter::optimize() {
}

string Formatter::print()  {
	ostringstream o;
	for(size_t i=0; i<mArray.size(); i++) {
		mArray[i].print(o);
	}
	return o.str();
}

void NodeAdder::addLinebreakNode() {
	mArray.alloc<LinebreakNode>();
}

// tag: pointer to tag in source data.
// len: length of tag in source data.
// tLen: length of basic tag (without attributes or whitespace)
// dst: static string; HTML representation of tag (without <>, so it can be used with attributes).
void NodeAdder::addTagNode(TagType type, const char* tag, size_t len, size_t tLen, const char* dst) {
	mArray.add(TagNode(tag, len, dst));
}

void NodeAdder::addColorTag(const char* id, size_t len) {
	mArray.add(ColorNode(id, len));
}

void NodeAdder::addWowfootUrlNode(const char* path, size_t len) {
	mArray.add(WowfootUrlNode(path, len));
}

void NodeAdder::addWowpediaUrlNode(const char* path, size_t len) {
	mArray.add(WowpediaUrlNode(path, len));
}

void NodeAdder::addUrlNode(const char* url, size_t len) {
	mArray.add(UrlNode(url, len));
}

void NodeAdder::addStaticTextNode(const char* text) {
	mArray.add(StaticTextNode(text));
}

void NodeAdder::addTextNode(const char* text, size_t len) {
	mArray.add(TextNode(text, len));
}

void NodeAdder::addUrlEndNode() {
	mArray.add(TagNode("/url", 4, "/a"));
}

template<class Map>
void NodeAdder::addPageNode(Map& map, const char* type, const char* id, size_t len)
{
	mArray.add(PageNode<Map>(map, type, id, len));
}

#define INSTANTIATE_ADDPAGENODE(name, map)\
	template void NodeAdder::addPageNode<typeof(map)>(typeof(map)&, const char* type, const char* id, size_t len);

PAGE_TAGS(INSTANTIATE_ADDPAGENODE);
