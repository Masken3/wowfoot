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
	void optimize();
	string printArray() const;
	void printNode(ostream&, int node) const;
	string printTree() const;

	int setupBasicNode(unsigned& i, int parent);
	void setupBasicTree();
	void dumpTreeNode(int level, int node) const;
	bool optimizeNode(int* n);

	// optimize subroutines
	Ref findChildTag(Ref, TagType type) const;
	Ref findNext2Li(Ref) const;
	Ref findLastSibling(Ref) const;
	bool tagIsEmpty(Ref) const;
	void updateTagCount(const Node& n, int baseDiff);

	int mTagCount[_TAG_TYPE_COUNT];
	const bool mLog;
};

#endif	//FORMATTER_H
