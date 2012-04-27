#include "formatter.h"
#include "node.h"
#include "pageTags.h"
#include "util/exception.h"
#include "util/minmax.h"
#include <sstream>
#include <string.h>
#include <stdio.h>

using namespace std;

#define LOG if(mLog) printf
#define DUMPINT(i) LOG(#i ": %i\n", i)

#define N REF(n)
#define RC REF(N.child)
#define VC VALID(N.child)
#define RN REF(N.next)
#define VN VALID(N.next)

struct Formatter::NodeStackFrame {
	Ref n;
	Ref Node::* mPtr;
	const NodeStackFrame* prevFrame;
};

string Formatter::formatComment(const char* src) {
	//printf("mLog: %i\n", mLog);
	// clear
	mFirstNode = INVALID;
	mPreviousNode = INVALID;
	mArray.clear();

	LOG("source: %s\n", src);
	parse(src);


	// cleanup after parsing
	// collapse any remaining node-stack frames
	Ref r = mFirstNode;
	while(VR) {
		mFirstNode = R.next;
		R.next = INVALID;
		printf("%i.next .= %i\n", r, INVALID);
		r = mFirstNode;
	}

	mFirstNode = 0;
	memset(mTagCount, 0, sizeof(mTagCount));

	optimize();
	//printf("mLog: %i\n", mLog);
	return printTree();
}

Formatter::Formatter(bool log)
: Parser(log)
{
}

static const bool sTagTypeAllowMultiple[_TAG_TYPE_COUNT] = {
#define _TAG_TYPE_ALLOW(name, allowMultiple, endTag) allowMultiple,
	TAG_TYPES(_TAG_TYPE_ALLOW)
#define _FORMATTING_TYPE_ALLOW(name, span, div) false,
	FORMATTING_TYPES(_FORMATTING_TYPE_ALLOW)
};

void Formatter::dumpTreeNode(int level, int n) const {
	if(!mLog)
		return;
	LOG("dumpTreeNode(%i, %i)\n", level, n);
	EASSERT(level < 16);
	while(VALID(n)) {
		N.dump(level);
		EASSERT(N.child > n || N.child < 0);
		EASSERT(N.next > n || N.next < 0);
		if(VALID(N.child))
			dumpTreeNode(level+1, N.child);
		n = N.next;
	}
}

void Formatter::optimize() {
	//setupBasicTree();
	dumpTreeNode(1, mFirstNode);

	LOG("optimizing...\n");
	NodeStackFrame nsf;
	nsf.n = INVALID;
	nsf.prevFrame = NULL;
	while(optimizeNode(nsf));
	dumpTreeNode(1, mFirstNode);

	// check that tag counts are valid (zero)
	for(int i=0; i<_TAG_TYPE_COUNT; i++) {
		if(mTagCount[i] != 0) {
			printf("mTagCount[%i]: %i\n", i, mTagCount[i]);
			FAIL("Bad tagCount");
		}
	}
}

// returns the Ref of an immediate child tag of the given root and type.
// returns INVALID if no such child exists.
// skips over whitespace nodes.
Ref Formatter::findChildTag(Ref root, TagType type) const {
	Ref r = REF(root).child;
	while(VR) {
		if(R.tagType() == type)
			return r;
		if(R.isLinebreak() || R.isSpace())
			r = R.next;
		else
			break;
	}
	return INVALID;
}

// returns the Ref of the first sibling of N such that a LIST_ITEM is its next,
// or INVALID if no such sibling exists.
Ref Formatter::findNext2Li(Ref n) const {
	Ref r = n;
	while(VR) {
		if(VALID(R.next) && REF(R.next).tagType() == LIST_ITEM)
			return r;
		r = R.next;
	}
	return INVALID;
}

Ref Formatter::findLastSibling(Ref n) const {
	Ref r = n;
	while(VR) {
		if(!VALID(R.next))
			return r;
		r = R.next;
	}
	return INVALID;
}

// returns true if n's children are all unprintable,
// or if there are no children.
bool Formatter::tagIsEmpty(Ref n) const {
	if(!N.isTag())
		return false;
	Ref r = N.child;
	while(VR) {
		if(!(R.isLinebreak() || R.isSpace()))
			return false;
		EASSERT(!VALID(R.child));
		r = R.next;
	}
	return true;
}

#define RESTART do { \
	if(N.tagType() == LIST_ITEM) { listItemCount--; } \
	goto loop; } while(0)

// set *np to remove the current tag.
bool Formatter::optimizeNode(const NodeStackFrame& nsf) {
	bool needAnotherRun = false;
	int originalTagCount[_TAG_TYPE_COUNT];
	int listItemCount = 1;
	memcpy(originalTagCount, mTagCount, sizeof(mTagCount));

	Ref* np;
	if(VALID(nsf.n))
		np = &(REF(nsf.n).*nsf.mPtr);
	else
		np = &mFirstNode;
	// walk the tree
	for(; VALID(*np); np = &REF(*np).next) {
		loop:
		Ref n = *np;

		if(VC) {
			// collapse outer [url] if UrlNode is inside.
			if(N.tagType() == ANCHOR && RC.hasUrl()) {
				LOG("collapsed outer url node: %i\n", n);
				*np = N.child;
				goto loop;
			}
		}

		// remove disallowed multiple tag
		if(N.isTag() &&
			!sTagTypeAllowMultiple[N.tagType()] &&
			mTagCount[N.tagType()] > 0)
		{
			LOG("removing multiple: %i (count[%i]: %i)\n", n, N.tagType(), mTagCount[N.tagType()]);
			if(VC)
				*np = N.child;
			else
				*np = N.next;
			if(!VALID(*np))
				break;
			goto loop;
		}

		// set list item value.
		// after this point, use RESTART instead of goto loop.
		if(N.tagType() == LIST_ITEM) {
			((ListItemNode&)N).value = listItemCount++;
		}

		if(VC) {
			EASSERT(N.isTag());
			{
				NodeStackFrame csf;
				csf.n = n;
				csf.mPtr = &Node::child;
				csf.prevFrame = &nsf;

				mTagCount[N.tagType()]++;
				while(optimizeNode(csf));
				mTagCount[N.tagType()]--;
			}
			if(!VC)
				RESTART;

			// remove duplicate [ul]
			Ref r;
			if(N.tagType() == LIST && (r = findChildTag(n, LIST)) != INVALID) {
				LOG("collapsed duplicate [ul]: %i\n", n);
				*np = r;
				RESTART;
			}

			// collapse [li] if outside LIST.
			if(N.tagType() == LIST_ITEM && mTagCount[LIST] <= mTagCount[LIST_ITEM]) {
				LOG("collapsed invalid [li]: %i (%i <= %i)\n",
					n, mTagCount[LIST], mTagCount[LIST_ITEM]);
				Ref next = N.next;	// the node after [/li]. may be INVALID.
				*np = N.child;
				r = findLastSibling(*np);
				R.next = next;
				RESTART;
			}
		}

		// inbetween LISTs and their ITEMs.
		if(N.tagType() == LIST) {
			handleList(N.child, n);
		} else if(N.tagType() == LIST_ITEM) {
			handleList(N.next, n);
		}

		if(VN) {
			// linebreak after [/ul].
			if(N.tagType() == LIST && RN.isLinebreak()) {
				((LinebreakNode&)RN).visible = false;
			}
		}

		// remove empty tag
		if(tagIsEmpty(n)) {
			LOG("removing empty: %i. Next: %i\n", n, N.next);
			*np = N.next;
			// if removed end tag has no sibling
			if(!VALID(*np)) {
				break;
			}
			// go back to previous sibling.
			// needed for example in this situation: ...[/li]\n[li][/li]\n
			// can't go back; whitespace nodes in the way.
			needAnotherRun = true;
			RESTART;
		}
	}
	for(int i=0; i<_TAG_TYPE_COUNT; i++) {
		if(mTagCount[i] != originalTagCount[i]) {
			printf("Error: o[%i]: %i => %i\n", i, originalTagCount[i], mTagCount[i]);
			FAIL("tagCount");
		}
	}
	return needAnotherRun;
}

void Formatter::handleList(Ref r, Ref prev) {
	while(VR) {
		// hide linebreaks.
		if(R.isLinebreak() || R.isSpace()) {
			if(R.isLinebreak())
				((LinebreakNode&)R).visible = false;
			prev = r;
			r = R.next;
			continue;
		} else if(R.tagType() == LIST_ITEM || R.tagType() == LIST) {
			break;
		}
		// add [li] around content nodes.
		LOG("Adding [li] around %i\n", r);
		Ref next2Li = findNext2Li(r);
		Ref nextLi = VALID(next2Li) ? REF(next2Li).next : INVALID;
		REF(prev).next = mArray.size();
		ListItemNode& t(mArray.add(ListItemNode()));
		t.styleNone = true;
		t._i = REF(prev).next;
		t.child = r;
		t.next = nextLi;
		if(VALID(next2Li))
			REF(next2Li).next = INVALID;
		r = next2Li;
		DUMPINT(next2Li);
		prev = r;
		if(VR)
			r = R.next;
	}
}

string Formatter::printArray() const {
	ostringstream o;
	for(size_t i=0; i<mArray.size(); i++) {
		mArray[i].print(o);
	}
	return o.str();
}

void Formatter::printNode(ostream& o, int n) const {
	while(VALID(n)) {
		N.print(o);
		printNode(o, N.child);
		N.printEndTag(o);
		n = N.next;
	}
}

string Formatter::printTree() const {
	ostringstream o;
	printNode(o, mFirstNode);
	return o.str();
}
