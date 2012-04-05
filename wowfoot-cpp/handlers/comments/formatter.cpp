#include "formatter.h"
#include "node.h"
#include "pageTags.h"
#include "util/exception.h"
#include "util/minmax.h"
#include <sstream>

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
			if(VALID(parent)) if(n.isEndTagOf(REF(parent))) {
				LOG("found end tag 1.\n");
				n.next = INVALID;
				n.child = INVALID;
				return i;
			}
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

void Formatter::optimizeNode(int* np) {
	// walk the tree
	for(; VALID(*np); np = &REF(*np).next) {
		int n = *np;
		if(VALID(REF(n).child)) {
			optimizeNode(&REF(n).child);
			if(!VALID(REF(n).child))
				continue;

			// collapse [url] if UrlNode is inside.
			if(REF(n).tagType() == ANCHOR && REF(REF(n).child).hasUrl()) {
				LOG("collapsed outer url node.\n");
				*np = REF(n).child;
				n = *np;
			}
			if(!VALID(REF(n).child))
				continue;

			// remove linebreaks between [ul] and [li]
			if(REF(n).tagType() == LIST && REF(REF(n).child).isLinebreak()) {
				((LinebreakNode&)REF(REF(n).child)).visible = false;
			}
		}
		if(VALID(REF(n).next)) {
			if(REF(n).tagType() == LIST_ITEM && REF(n).isEndTag() && REF(REF(n).next).isLinebreak()) {
				((LinebreakNode&)REF(REF(n).next)).visible = false;
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
		REF(n).print(o);
		printNode(o, REF(n).child);
		n = REF(n).next;
	}
}

string Formatter::printTree() {
	ostringstream o;
	printNode(o, mFirstNode);
	return o.str();
}
