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

// returns the value of parent.next,
// unless parent is NULL, in which case the return value is undefined.
Node* Formatter::setupBasicNode(size_t& i, const Node* parent) {
	LOG("setupBasicNode(%zu, %p)\n", i, parent);
	for(; i<mArray.size(); i++) {
		LOG("node %zu\n", i);
		Node& n(mArray[i]);
		n._i = i;
		if(n.isTag()) {
			LOG("isTag. parent: %p\n", parent);
			if(parent) if(n.isEndTagOf(*parent)) {
				LOG("found end tag 1.\n");
				n.next = NULL;
				n.child = NULL;
				return &n;
			}
			if(i == mArray.size()-1)
				break;
			Node& m(mArray[i+1]);
			if(parent) if(m.isEndTagOf(*parent)) {
				n.child = NULL;
				n.next = NULL;
				return &m;
			}
			if(m.isEndTagOf(n)) {
				LOG("is end tag.\n");
				n.child = NULL;
				n.next = &m;
			} else if(n.isEndTag()) {
				n.child = NULL;
				n.next = &m;
			} else {
				LOG("has child.\n");
				n.child = &m;
				i++;
				n.next = setupBasicNode(i, &n);
			}
		} else {
			if(i == mArray.size()-1)
				break;
			n.child = NULL;
			Node& m(mArray[i+1]);
			if(parent) if(m.isEndTagOf(*parent)) {
				LOG("found end tag 2.\n");
				n.next = NULL;
				return &m;
			}
			n.next = &m;
		}
	}
	LOG("return NULL;\n");
	return NULL;
}

void Formatter::setupBasicTree() {
	// walk the array
	assert(mArray.size() > 0);
	mFirstNode = &mArray[0];
	size_t i=0;
	setupBasicNode(i, NULL);
	Node& n(mArray[mArray.size()-1]);
	n.child = NULL;
	n.next = NULL;
}

void Formatter::dumpTreeNode(int level, const Node* n) {
	while(n) {
		//LOG("dumpTreeNode(%i, %p)\n", level, n);
		n->dump(level);
		if(n->child)
			dumpTreeNode(level+1, n->child);
		n = n->next;
	}
}

void Formatter::optimize() {
	setupBasicTree();
	dumpTreeNode(1, mFirstNode);

	LOG("optimizing...\n");
	optimizeNode(&mFirstNode);
	dumpTreeNode(1, mFirstNode);
}

void Formatter::optimizeNode(Node** np) {
	// walk the tree
	while(*np) {
		Node& n(**np);
		if(n.child) {
			optimizeNode(&n.child);

			// collapse [url] if UrlNode is inside.
			if(n.tagType() == ANCHOR && n.child->hasUrl()) {
				LOG("collapsed outer url node.\n");
				*np = n.child;
			}

			// remove linebreaks between [ul] and [li]
			if(n.tagType() == LIST && n.child->isLinebreak()) {
				((LinebreakNode*)n.child)->visible = false;
			}
		}
		if(n.next) {
			if(n.tagType() == LIST_ITEM && n.isEndTag() && n.next->isLinebreak()) {
				((LinebreakNode*)n.next)->visible = false;
			}
		}

		np = &(*np)->next;
	}
}

string Formatter::printArray()  {
	ostringstream o;
	for(size_t i=0; i<mArray.size(); i++) {
		mArray[i].print(o);
	}
	return o.str();
}

void Formatter::printNode(ostream& o, const Node* n) {
	while(n) {
		n->print(o);
		printNode(o, n->child);
		n = n->next;
	}
}

string Formatter::printTree() {
	ostringstream o;
	printNode(o, mFirstNode);
	return o.str();
}
