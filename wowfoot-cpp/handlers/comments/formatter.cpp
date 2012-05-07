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
	if(mLog) {
		FILE* f = fopen("comment_source.txt", "wb");
		fwrite(src, strlen(src), 1, f);
		fclose(f);
	}
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
	//LOG("dumpTreeNode(%i, %i)\n", level, n);
	EASSERT(level < 16);
	while(VALID(n)) {
		N.dump(level);
		//EASSERT(N.child > n || N.child < 0);
		//EASSERT(N.next > n || N.next < 0);
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
	int o;
	while((o = optimizeNode(nsf)) == 0);
	EASSERT(o < 0);
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

void Formatter::dumpNodeStack(const NodeStackFrame& nsf) const {
	LOG(" %i", VALID(nsf.n) ? nsf.n : mFirstNode);
	if(nsf.prevFrame)
		dumpNodeStack(*nsf.prevFrame);
}

// set *np to remove the current tag.
// returns the number of levels that need rerunning.
// <0 means none, 0 means the child, >0 means 1 or more parent levels.
int Formatter::optimizeNode(const NodeStackFrame& nsf) {
	bool rerunChild = false;
	int rerunParent = 0;
	int originalTagCount[_TAG_TYPE_COUNT];
	int listItemCount = 1;
	memcpy(originalTagCount, mTagCount, sizeof(mTagCount));
	//dumpNodeStack(nsf);
	//LOG("\n");

	NodeStackFrame localNsf = nsf;
#define NR (VALID(localNsf.n) ? (REF(localNsf.n).*localNsf.mPtr) : mFirstNode)
	// walk the tree
	for(; VALID(NR); localNsf.n = NR, localNsf.mPtr = &Node::next) {
		loop:
		Ref n = NR;
		if(!VALID(n))
			break;

		if(VC) {
			// collapse outer [url] if UrlNode is inside.
			if(N.tagType() == ANCHOR && RC.hasUrl()) {
				LOG("collapsed outer url node: %i\n", n);
				NR = N.child;
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
				NR = N.child;
			else
				NR = N.next;
			if(!VALID(NR))
				break;
			goto loop;
		}

		// set list item value.
		// after this point, use RESTART instead of goto loop.
		if(N.tagType() == LIST_ITEM) {
			ListItemNode& lin((ListItemNode&)N);
			if(!lin.styleNone)
				lin.value = listItemCount++;
		}

		if(VC) {
			// remove duplicate [ul]
			Ref r;
			if(N.tagType() == LIST && (r = findChildTag(n, LIST)) != INVALID) {
				LOG("collapsed duplicate [ul]: %i\n", n);
				NR = r;
				RESTART;
			}

			// collapse [li] if outside LIST.
			if(N.tagType() == LIST_ITEM && mTagCount[LIST] <= mTagCount[LIST_ITEM]) {
				LOG("collapsed invalid [li]: %i (%i <= %i)\n",
					n, mTagCount[LIST], mTagCount[LIST_ITEM]);
				Ref next = N.next;	// the node after [/li]. may be INVALID.
				NR = N.child;
				r = findLastSibling(NR);
				R.next = next;
				RESTART;
			}

			// run the child node
			EASSERT(N.isTag());
			{
				NodeStackFrame csf;
				csf.n = n;
				csf.mPtr = &Node::child;
				csf.prevFrame = &nsf;

#define DUMP_NODES 0
#if DUMP_NODES
	LOG("s");
	dumpNodeStack(nsf);
	LOG("\n");
#endif
				mTagCount[N.tagType()]++;
				int o;
				while((o = optimizeNode(csf)) == 0);
				mTagCount[N.tagType()]--;
#if DUMP_NODES
	LOG("e");
	dumpNodeStack(nsf);
	LOG("\n");
#endif
				if(o == 1) {
					LOG("oN restart\n");
					RESTART;
				}
				if(o > 1)
					return o-1;
			}
			if(!VC) {
				LOG("oN VC restart\n");
				RESTART;
			}

			// hide linebreaks after structure tags.
			if(N.isStructureTag() && RC.isLinebreak()) {
				((LinebreakNode&)RC).visible = false;
			}
		}

		// inbetween LISTs and their ITEMs.
		bool listed = false;
		if(N.tagType() == LIST) {
			listed = handleList(n, &Node::child);
		} else if(N.tagType() == LIST_ITEM) {
			listed = handleList(n, &Node::next);
		}
		if(listed) {
			LOG("listed RESTART\n");
			RESTART;
		}

		// remove empty tag
		if(tagIsEmpty(n)) {
			LOG("removing empty: %i. Next: %i\n", n, N.next);
			NR = N.next;
			// if removed end tag has no sibling
			if(!VALID(NR)) {
				break;
			}
			// go back to previous sibling.
			// needed for example in this situation: ...[/li]\n[li][/li]\n
			// can't go back; whitespace nodes in the way.
			rerunChild = true;
			RESTART;
		}

		if(VN) {
			// hide linebreaks after structure tags.
			if(N.isStructureTag() && RN.isLinebreak()) {
				((LinebreakNode&)RN).visible = false;
			}
		}

		// spans are not allowed around structure tags.
		// close them and replace with divs.
		if(N.isStructureTag()) {
			// check the node stack
			bool foundFormattingTag = false;
			const NodeStackFrame* nsfp = &nsf;
			Ref r;
			Ref newChild = n;
			int rp = 0;
			while(VALID(r = nsfp->n)) {
				// let's assume no other tags between N and formatting.
#if 0
				if(IS_STRUCTURE_TAG(R))
					break;
				rp++;
				if(R.isFormattingTag()) {
#else
				if(!R.isFormattingTag()) {
					break;
				} else {
#endif
					BaseFormattingNode& bfn((BaseFormattingNode&)R);
					if(bfn.div)
						break;
					LOG("found formatting tag %i outside %i\n", r, n);
					foundFormattingTag = true;
					rerunParent++;
					EASSERT(!bfn.div);
					// add div
					int i = mArray.size();
					BaseFormattingNode& t(mArray.add(bfn));
					t.div = true;
					t._i = i;
					t.child = newChild;
					newChild = i;
					t.next = bfn.next;
					// fix span
					NR = INVALID;
					bfn.next = i;
					// todo: add spans after div
				}
				nsfp = nsfp->prevFrame;
				EASSERT(nsfp);
			}
			if(foundFormattingTag)
				break;
			else
				rp = 0;
			if(rp > rerunParent)
				rerunParent = rp;
		}
	}
	for(int i=0; i<_TAG_TYPE_COUNT; i++) {
		if(mTagCount[i] != originalTagCount[i]) {
			printf("Error: o[%i]: %i => %i\n", i, originalTagCount[i], mTagCount[i]);
			FAIL("tagCount");
		}
	}
	return rerunParent ? rerunParent : (rerunChild ? 0 : -1);
}

bool Formatter::handleList(Ref n, Ref Node::* mPtr) {
	Ref r = N.*mPtr;
	bool listed = false;
	while(VR) {
		// hide linebreaks.
		if(R.isLinebreak() || R.isSpace()) {
			if(R.isLinebreak())
				((LinebreakNode&)R).visible = false;
			n = r;
			mPtr = &Node::next;
			r = R.next;
			continue;
		} else if(R.tagType() == LIST) {
			// collapse invalid LIST nodes.
			LOG("collapsed invalid [ul]: %i\n", r);
			N.*mPtr = r = R.child;
			listed = true;
			continue;
		} else if(R.tagType() == LIST_ITEM) {
			break;
		}
		// add [li] around content nodes.
		LOG("Adding [li] around %i\n", r);
		Ref next2Li = findNext2Li(r);
		Ref nextLi = VALID(next2Li) ? REF(next2Li).next : INVALID;
		N.*mPtr = mArray.size();
		// would invalidate np.
		ListItemNode& t(mArray.add(ListItemNode()));
		t.styleNone = true;
		t._i = N.*mPtr;
		t.child = r;
		t.next = nextLi;
		if(VALID(next2Li))
			REF(next2Li).next = INVALID;
		r = next2Li;
		DUMPINT(next2Li);
		n = r;
		mPtr = &Node::next;
		if(VR)
			r = R.next;
		listed = true;
	}
	return listed;
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
