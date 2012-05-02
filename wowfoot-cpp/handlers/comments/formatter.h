#ifndef FORMATTER_H
#define FORMATTER_H

#include <string>
#include "parser.h"
#include "vtree.h"

using namespace std;

class Formatter : Parser {
public:
	Formatter(bool log);
	string formatComment(const char* src);
private:
	struct NodeStackFrame;

	void optimize();
	string printArray() const;
	void printNode(ostream&, int node) const;
	string printTree() const;

	int setupBasicNode(unsigned& i, int parent);
	void setupBasicTree();
	void dumpTreeNode(int level, int node) const;
	int optimizeNode(const NodeStackFrame&);

	// optimize subroutines
	void dumpNodeStack(const NodeStackFrame& nsf) const;
	Ref findChildTag(Ref, TagType type) const;
	Ref findNext2Li(Ref) const;
	Ref findLastSibling(Ref) const;
	bool tagIsEmpty(Ref) const;
	void handleList(Ref, Ref Node::*);

	int mTagCount[_TAG_TYPE_COUNT];
};

#endif	//FORMATTER_H
