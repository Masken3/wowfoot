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

Formatter::Formatter(bool log)
: mLog(log)
{
}

static const bool sTagTypeAllowMultiple[] = {
#define _TAG_TYPE_ALLOW(name, allowMultiple, endTag) allowMultiple,
	TAG_TYPES(_TAG_TYPE_ALLOW)
};

void Formatter::dumpTreeNode(int level, int n) const {
	if(!mLog)
		return;
	LOG("dumpTreeNode(%i, %i)\n", level, n);
	EASSERT(level < 16);
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

	// check that tag counts are valid (zero)
	for(int i=0; i<_TAG_TYPE_COUNT; i++) {
		if(mTagCount[i] != 0) {
			printf("mTagCount[%i]: %i\n", i, mTagCount[i]);
			FAIL("Bad tagCount");
		}
	}
}

#define N REF(n)
#define RC REF(N.child)
#define VC VALID(N.child)
#define RN REF(N.next)
#define VN VALID(N.next)

// returns the Ref of an immediate child tag of the given root and type.
// returns INVALID if no such child exists.
// skips over Linebreak nodes, but no others.
Formatter::Ref Formatter::findChildTag(Ref root, TagType type) const {
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
Formatter::Ref Formatter::findNext2Li(Ref n) const {
	Ref r = n;
	while(VR) {
		if(VALID(R.next) && REF(R.next).tagType() == LIST_ITEM)
			return r;
		r = R.next;
	}
	return INVALID;
}

Formatter::Ref Formatter::findLastSibling(Ref n) const {
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
	Ref r = N.child;
	EASSERT(N.isTag() && !N.isEndTag());
	while(VR) {
		if(!(R.isLinebreak() || R.isSpace()))
			return false;
		EASSERT(!VALID(R.child));
		r = R.next;
	}
	return true;
}

void Formatter::updateTagCount(const Node& n, int baseDiff) {
	if(n.isTag()) {
		int diff = n.isEndTag() ? -baseDiff : baseDiff;
		mTagCount[n.tagType()] += diff;
		LOG("tag count %i: %s%i (v %i, n %i)\n",
			n.tagType(), diff > 0 ? "+" : "", diff, mTagCount[n.tagType()], n._i);
	}
}

#define RESTART do { updateTagCount(N, -1); goto loop; } while(0)

// set *np to remove the current tag.
void Formatter::optimizeNode(Ref* np) {
	int originalTagCount[_TAG_TYPE_COUNT];
	memcpy(originalTagCount, mTagCount, sizeof(mTagCount));
	// walk the tree
	for(; VALID(*np); np = &REF(*np).next) {
		loop:
		Ref n = *np;

		// remove disallowed multiple tag
		if(N.isTag() && !N.isEndTag() &&
			!sTagTypeAllowMultiple[N.tagType()] && mTagCount[N.tagType()] > 0)
		{
			LOG("removing multiple: %i (count[%i]: %i)\n", n, N.tagType(), mTagCount[N.tagType()]);
			if(VC)
				*np = N.child;
			else if(VN)
				*np = RN.next;
			if(!VALID(*np))
				break;
			goto loop;
		}

		// update tag count
		updateTagCount(N, 1);

		if(VC) {
			optimizeNode(&N.child);
			if(!VC)
				RESTART;

			// collapse outer [url] if UrlNode is inside.
			if(N.tagType() == ANCHOR && RC.hasUrl()) {
				LOG("collapsed outer url node: %i\n", n);
				*np = N.child;
				RESTART;
			}

			// remove duplicate [ul]
			Ref r;
			if(N.tagType() == LIST && (r = findChildTag(n, LIST)) != INVALID) {
				LOG("collapsed duplicate [ul]: %i\n", n);
				*np = r;
				RESTART;
			}

			// collapse [li] if outside LIST.
			if(N.tagType() == LIST_ITEM && mTagCount[LIST] < mTagCount[LIST_ITEM]) {
				LOG("collapsed invalid [li]: %i (%i < %i)\n",
					n, mTagCount[LIST], mTagCount[LIST_ITEM]);
				EASSERT(VN);
				EASSERT(RN.tagType() == LIST_ITEM && RN.isEndTag());
				EASSERT(!VALID(RN.child));	// end tags may not have a child node.
				Ref next = RN.next;	// the node after [/li]. may be INVALID.
				*np = N.child;
				r = findLastSibling(*np);
				R.next = next;
				RESTART;
			}

			// hide linebreak between [ul] and [li]
			if(N.tagType() == LIST && RC.isLinebreak()) {
				((LinebreakNode&)RC).visible = false;
			}
		}
		if(VN) {
			// deal with what comes after [/li].
			if(N.tagType() == LIST_ITEM && N.isEndTag()) {
				Ref r = N.next;
				Ref prev = n;
				do {
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
					Node& t(mArray.add(TagNode("li", 2, LIST_ITEM, "li", "/li")));
					t._i = REF(prev).next;
					t.child = r;
					t.next = mArray.size();
					Node& e(mArray.add(TagNode("/li", 3, LIST_ITEM, "/li", NULL)));
					e._i = t.next;
					e.child = INVALID;
					e.next = nextLi;
					if(VALID(next2Li))
						REF(next2Li).next = INVALID;
					r = next2Li;
					DUMPINT(next2Li);
					prev = r;
					if(VR)
						r = R.next;
				} while(VR);
			}
			// linebreak after LIST.
			if(N.tagType() == LIST && N.isEndTag() && RN.isLinebreak()) {
				((LinebreakNode&)RN).visible = false;
			}

			// remove empty tag
			if(RN.isEndTagOf(N) && tagIsEmpty(n)) {
				LOG("removing empty: %i. Next: %i\n", n, RN.next);
				*np = RN.next;
				if(!VALID(*np)) {
					mTagCount[N.tagType()]--;
					break;
				}
				RESTART;
			}
		}
	}
	for(int i=0; i<_TAG_TYPE_COUNT; i++) {
		if(mTagCount[i] != originalTagCount[i]) {
			printf("Error: o[%i]: %i => %i\n", i, originalTagCount[i], mTagCount[i]);
			FAIL("tagCount");
		}
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
		n = N.next;
	}
}

string Formatter::printTree() const {
	ostringstream o;
	printNode(o, mFirstNode);
	return o.str();
}
