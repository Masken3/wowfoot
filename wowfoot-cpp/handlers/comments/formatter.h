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

	Ref findChildTag(Ref root, TagType type);

	int mTagCount[_TAG_TYPE_COUNT];
};

#endif	//FORMATTER_H
