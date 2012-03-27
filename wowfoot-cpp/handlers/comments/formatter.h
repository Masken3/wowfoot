#ifndef FORMATTER_H
#define FORMATTER_H

#include "parser.h"
#include "vtree.h"
#include <string>

using namespace std;

class Formatter : Parser {
public:
	Formatter();
	string formatComment(const char* src);
private:
	void optimize();
	string print();
};

#endif	//FORMATTER_H
