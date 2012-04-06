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
	Formatter f;
	LOG("source: %s\n", src);
	f.parse(src);
	f.optimize();
	return f.printTree();
}

Formatter::Formatter() {
}

#define REF(i) mArray[i]
#define VALID(i) (i >= 0)
#define INVALID (-1)

static const bool sTagTypeAllowMultiple[] = {
#define _TAG_TYPE_ALLOW(name, allowMultiple, endTag) allowMultiple,
	TAG_TYPES(_TAG_TYPE_ALLOW)
};

// returns the value of parent.next,
// unless parent is INVALID, in which case the return value is undefined.
int Formatter::setupBasicNode(unsigned& i, int parent) {
	LOG("setupBasicNode(%u, %i)\n", i, parent);
	for(; i<mArray.size(); i++) {
		LOG("node %u\n", i);
		Node& n(mArray[i]);
		n._i = i;
		if(n.isTag()) {
			LOG("isTag. parent: %i\n", parent);
			// parenting
			if(VALID(parent)) if(n.isEndTagOf(REF(parent))) {
				LOG("found end tag 1.\n");
				n.next = INVALID;
				n.child = INVALID;
				return i;
			}

			// next node: child or sibling?
			if(i == mArray.size()-1)
				break;
			Node& m(mArray[i+1]);
			if(VALID(parent)) if(m.isEndTagOf(REF(parent))) {
				n.child = INVALID;
				n.next = INVALID;
				return i+1;
			}
			if(m.isEndTagOf(n)) {
				LOG("is end tag.\n");
				n.child = INVALID;
				n.next = i+1;
			} else if(n.isEndTag()) {
				n.child = INVALID;
				n.next = i+1;
			} else {
				LOG("has child.\n");
				i++;
				n.child = i;
				n.next = setupBasicNode(i, i-1);
				if(!VALID(n.next)) {
					n.next = (int)mArray.size();
					addTagNode(n.tagType(), NULL, 0, 0, n.endTag(), NULL);
				}
			}
		} else {
			if(i == mArray.size()-1)
				break;
			n.child = INVALID;
			Node& m(mArray[i+1]);
			if(VALID(parent)) if(m.isEndTagOf(REF(parent))) {
				LOG("found end tag 2.\n");
				n.next = INVALID;
				return i+1;
			}
			n.next = i+1;
		}
	}
	LOG("return INVALID;\n");
	return INVALID;
}

void Formatter::setupBasicTree() {
	// walk the array
	assert(mArray.size() > 0);
	mFirstNode = 0;
	memset(mTagCount, 0, sizeof(mTagCount));
	unsigned i=0;
	setupBasicNode(i, INVALID);
	Node& n(mArray[mArray.size()-1]);
	n.child = INVALID;
	n.next = INVALID;
}

void Formatter::dumpTreeNode(int level, int n) {
	while(VALID(n)) {
		//LOG("dumpTreeNode(%i, %p)\n", level, n);
		REF(n).dump(level);
		if(VALID(REF(n).child))
			dumpTreeNode(level+1, REF(n).child);
		n = REF(n).next;
	}
}

void Formatter::optimize() {
	setupBasicTree();
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

// set *np to remove the current tag.
void Formatter::optimizeNode(int* np) {
	// walk the tree
	for(; VALID(*np); np = &REF(*np).next) {
		int n = *np;

		// remove disallowed multiple tag
		if(N.isTag() && !N.isEndTag() &&
			!sTagTypeAllowMultiple[N.tagType()] && mTagCount[N.tagType()] > 0)
		{
			*np = N.child;
			continue;
		}

		if(VC) {
			// update tag count
			if(N.isTag()) {
				int diff = N.isEndTag() ? -1 : 1;
				mTagCount[N.tagType()] += diff;
			}

			optimizeNode(&N.child);
			if(!VC)
				continue;

			// collapse [url] if UrlNode is inside.
			if(N.tagType() == ANCHOR && RC.hasUrl()) {
				LOG("collapsed outer url node.\n");
				*np = N.child;
				n = *np;
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
