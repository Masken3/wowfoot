#ifndef FORMATTER_H
#define FORMATTER_H

#include "parser.h"
#include "vtree.h"
#include <string>

using namespace std;

class Formatter : Parser {
public:
	Formatter();
	static string formatComment(const char* src);
private:
	void optimize();
	string printArray();
	void printNode(ostream&, const Node*);
	string printTree();

	Node* setupBasicNode(size_t& i, const Node* parent);
	void setupBasicTree();
	void dumpTreeNode(int level, const Node*);

	Node* mFirstNode;
};

#endif	//FORMATTER_H
