#include "parser.h"
#include "node.h"
#include "pageTags.h"

NodeAdder::Ref NodeAdder::findStartTag(const Node& endTag) {
	Ref r = mFirstNode;
	while(VALID(r)) {
		if(endTag.isEndTagOf(R))
			return r;
		r = R.next;
	}
	return INVALID;
}

// during the parsing step, mFirstNode refs the top of the tag stack.
// each stack frame (start tag) has 'next' ref its parent.
void NodeAdder::add(const Node& n) {
	//n.dump(0);
	Ref newPrev;
	if(n.isEndTag()) {
		// if there is no start tag, discard this end tag.
		Ref s = findStartTag(n);
		if(!VALID(s))
			return;
		// close all tags in between.
		newPrev = s+1;
		Ref r;
		do {
			r = mFirstNode;
			mFirstNode = R.next;
			R.next = r+1;
			//printf("%i.next .= %i\n", r, r+1);
		} while(r != s);
		mPreviousNode = INVALID;
	} else {
		newPrev = mArray.size();
		Node& t(mArray.add(n));
		t._i = newPrev;
		//t.dump(0);
		if(n.isTag()) {
			// this is the most recent parent; add it to the stack.
			t.next = mFirstNode;
			//printf("%i.next := %i\n", newPrev, mFirstNode);
			mFirstNode = newPrev;
			// add n's end tag.
			Node& e(mArray.add(TagNode(t.endTag(), strlen(t.endTag()), t.tagType(), t.endTag(), NULL)));
			e._i = newPrev+1;
			//e.dump(0);
		}
	}
	// set refs.
	if(VALID(mPreviousNode)) {
		Node& p(REF(mPreviousNode));
		if(p.isTag() && !p.isEndTag()) {
			//printf("%i.child = %i\n", mPreviousNode, newPrev);
			p.child = newPrev;
		} else {
			//printf("%i.next = %i\n", mPreviousNode, newPrev);
			p.next = newPrev;
		}
		p._i = mPreviousNode;
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
	ADD(TagNode(tag, len, type, dst, end));
}

void NodeAdder::addUrlEndNode() {
	addTagNode(ANCHOR, "/url", 4, 4, "/a", NULL);
}

void NodeAdder::addFormattingTag(FormattingType type) {
	ADD(FormattingNode(type));
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
