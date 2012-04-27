#include "parser.h"
#include "node.h"
#include "pageTags.h"
#include <stdio.h>

#define LOG if(mLog) printf

Ref NodeAdder::findStartTag(const char* endTag) {
	Ref r = mFirstNode;
	while(VALID(r)) {
		if(R.hasEndTag(endTag))
			return r;
		r = R.next;
	}
	return INVALID;
}

// during the parsing step, mFirstNode refs the top of the tag stack.
// each stack frame (start tag) has 'next' ref its parent.
void NodeAdder::add(const Node& n) {
	//n.dump(0);
	Ref newPrev = mArray.size();
	Node& t(mArray.add(n));
	t._i = newPrev;
	if(mLog)
		t.dump(0);
	if(n.isTag()) {
		// this is the most recent parent; add it to the stack.
		t.next = mFirstNode;
		LOG("%i.next := %i\n", newPrev, mFirstNode);
		mFirstNode = newPrev;
		//e.dump(0);
	}
	setRefs(newPrev, false);
}

void NodeAdder::addEndTag(const char* endTag) {
	// if there is no start tag, discard this end tag.
	Ref s = findStartTag(endTag);
	if(!VALID(s))
		return;
	LOG("endTag %s to %i\n", endTag, s);
	// close all tags in between.
	Ref r;
	do {
		r = mFirstNode;
		mFirstNode = R.next;
		R.next = INVALID;
		LOG("%i.next .= %i\n", r, INVALID);
	} while(r != s);
	setRefs(INVALID, true);
	mPreviousNode = r;
}

void NodeAdder::setRefs(Ref newPrev, bool end) {
	// set refs.
	if(VALID(mPreviousNode)) {
		Node& p(REF(mPreviousNode));
		if(p.isTag() && !end && p.child == UNDEFINED) {
			LOG("%i.child = %i\n", mPreviousNode, newPrev);
			p.child = newPrev;
		} else {
			if(p.child == UNDEFINED)
				p.child = INVALID;
			LOG("%i.next = %i\n", mPreviousNode, newPrev);
			p.next = newPrev;
		}
	}
	mPreviousNode = newPrev;
}

#define ADD(n) add(n)

void NodeAdder::addLinebreakNode() {
	ADD(LinebreakNode(true));
}

// tag: pointer to tag in source data.
// len: length of tag in source data.
// tLen: length of basic tag (without attributes or whitespace)
// dst: static string; HTML representation of tag (without <>, so it can be used with attributes).
void NodeAdder::addTagNode(TagType type, const char* tag, size_t len, size_t tLen, const char* dst, const char* end) {
	ADD(TagNode(tag, len, tLen, type, dst, end));
}

void NodeAdder::addUrlEndNode() {
	addEndTag("/url");
}

void NodeAdder::addFormattingTag(const char* tag, size_t len, size_t tLen, FormattingType type) {
	ADD(FormattingNode(tag, len, tLen, type));
}

void NodeAdder::addColorTag(const char* id, size_t len) {
	ADD(ColorNode(id, len));
}

void NodeAdder::addWowfootUrlNode(const char* path, size_t len) {
	ADD(WowfootUrlNode(path, len));
}

void NodeAdder::addWowpediaUrlNode(const char* path, size_t len) {
	ADD(WowpediaUrlNode(path, len));
}

void NodeAdder::addUrlNode(const char* url, size_t len) {
	ADD(UrlNode(url, len));
}

void NodeAdder::addStaticTextNode(const char* text) {
	ADD(StaticTextNode(text));
}

void NodeAdder::addTextNode(const char* text, size_t len) {
	ADD(TextNode(text, len));
}

void NodeAdder::addListItem() {
	ADD(ListItemNode());
}

template<class Map>
void NodeAdder::addPageNode(Map& map, const char* type, const char* id, size_t len)
{
	ADD(PageNode<Map>(map, type, id, len));
}

#define INSTANTIATE_ADDPAGENODE(name, map)\
	template void NodeAdder::addPageNode<typeof(map)>(typeof(map)&, const char* type, const char* id, size_t len);

PAGE_TAGS(INSTANTIATE_ADDPAGENODE);
