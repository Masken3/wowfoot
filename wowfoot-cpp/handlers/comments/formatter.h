#ifndef FORMATTER_H
#define FORMATTER_H

#include <string>
#include "parser.h"
#include "vtree.h"

using namespace std;

class Formatter : Parser {
public:
	Formatter();
	string formatComment(const char* src);
private:
	void optimize();
	string printArray();
	void printNode(ostream&, int node);
	string printTree();

	int setupBasicNode(unsigned& i, int parent);
	void setupBasicTree();
	void dumpTreeNode(int level, int node);
	void optimizeNode(int* n);

	// optimize subroutines
	Ref findChildTag(Ref, TagType type);
	Ref findNext2Li(Ref);
	Ref findLastSibling(Ref);
	void updateTagCount(const Node& n, int baseDiff);

	int mTagCount[_TAG_TYPE_COUNT];
};

#endif	//FORMATTER_H
