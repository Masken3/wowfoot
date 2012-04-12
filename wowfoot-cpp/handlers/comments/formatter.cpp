#include "formatter.h"
#include "node.h"
#include "pageTags.h"
#include "util/exception.h"
#include "util/minmax.h"
#include <sstream>
#include <string.h>

using namespace std;

#define LOG printf

string Formatter::formatComment(const char* src) {
	// clear
	mFirstNode = INVALID;
	mPreviousNode = INVALID;
	mArray.clear();

	LOG("source: %s\n", src);
	parse(src);

	// cleanup after parsing
	// collapse any remaining node-stack frames
#define R REF(r)
	Ref r = mFirstNode;
	while(VALID(r)) {
		mFirstNode = R.next;
		R.next = r+1;
		printf("%i.next .= %i\n", r, r+1);
		r = mFirstNode;
	}

	mFirstNode = 0;
	memset(mTagCount, 0, sizeof(mTagCount));

	optimize();
	return printTree();
}

Formatter::Formatter() {
}

static const bool sTagTypeAllowMultiple[] = {
#define _TAG_TYPE_ALLOW(name, allowMultiple, endTag) allowMultiple,
	TAG_TYPES(_TAG_TYPE_ALLOW)
};

void Formatter::dumpTreeNode(int level, int n) {
	LOG("dumpTreeNode(%i, %i)\n", level, n);
	EASSERT(level < 8);
	while(VALID(n)) {
		REF(n).dump(level);
		if(VALID(REF(n).child))
			dumpTreeNode(level+1, REF(n).child);
		n = REF(n).next;
	}
}

void Formatter::optimize() {
	//setupBasicTree();
	dumpTreeNode(1, mFirstNode);

	LOG("optimizing...\n");
	optimizeNode(&mFirstNode);
	dumpTreeNode(1, mFirstNode);
}

#define N REF(n)
#define RC REF(N.child)
#define VC VALID(N.child)
#define RN REF(N.next)
#define VN VALID(N.next)

// returns the Ref of an immediate child tag of the given root and type.
// returns INVALID if no such child exists.
// skips over Linebreak nodes, but no others.
Formatter::Ref Formatter::findChildTag(Ref root, TagType type) {
	Ref c = REF(root).child;
	while(VALID(c)) {
		if(REF(c).tagType() == type)
			return c;
		if(REF(c).isLinebreak())
			c = REF(c).next;
		else
			break;
	}
	return INVALID;
}

// set *np to remove the current tag.
void Formatter::optimizeNode(Ref* np) {
	// walk the tree
	for(; VALID(*np); np = &REF(*np).next) {
		Ref n = *np;

		// remove disallowed multiple tag
		if(N.isTag() && !N.isEndTag() &&
			!sTagTypeAllowMultiple[N.tagType()] && mTagCount[N.tagType()] > 0)
		{
			LOG("removing multiple: %i (count[%i]: %i)\n", n, N.tagType(), mTagCount[N.tagType()]);
			*np = N.child;
			continue;
		}

		// update tag count
		if(N.isTag()) {
			int diff = N.isEndTag() ? -1 : 1;
			//LOG("tag count %i: %s%i\n", N.tagType(), diff > 0 ? "+" : "", diff);
			mTagCount[N.tagType()] += diff;
		}

		if(VC) {
			optimizeNode(&N.child);
			if(!VC)
				continue;

			// collapse [url] if UrlNode is inside.
			if(N.tagType() == ANCHOR && RC.hasUrl()) {
				LOG("collapsed outer url node: %i\n", n);
				*np = N.child;
				n = *np;
			}
			if(!VC)
				continue;

			// remove duplicate [ul]
			Ref r;
			if(N.tagType() == LIST && (r = findChildTag(n, LIST)) != INVALID) {
				n = *np = r;
			}
			if(!VC)
				continue;

			// remove linebreak between [ul] and [li]
			if(N.tagType() == LIST && RC.isLinebreak()) {
				((LinebreakNode&)RC).visible = false;
			}
		}
		if(VN) {
			// remove linebreak between [/li] and [li]
			if(N.tagType() == LIST_ITEM && N.isEndTag() && RN.isLinebreak()) {
				((LinebreakNode&)RN).visible = false;
			}

			// remove empty tag
			if(RN.isEndTagOf(N) && !VC) {
				LOG("removing empty: %i\n", n);
				*np = RN.next;
				if(!VALID(*np))
					break;
			}
		}
	}
}

string Formatter::printArray()  {
	ostringstream o;
	for(size_t i=0; i<mArray.size(); i++) {
		mArray[i].print(o);
	}
	return o.str();
}

void Formatter::printNode(ostream& o, int n) {
	while(VALID(n)) {
		N.print(o);
		printNode(o, N.child);
		n = N.next;
	}
}

string Formatter::printTree() {
	ostringstream o;
	printNode(o, mFirstNode);
	return o.str();
}
