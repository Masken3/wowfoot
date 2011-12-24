#ifndef ITEM_SUB_CLASS_H
#define ITEM_SUB_CLASS_H

#include "dllHelpers.h"
#include <vector>

using namespace std;

struct ItemSubClass {
	int hands;
	const char* name;
	const char* plural;
};

class ItemSubClasses {
private:
	vector<vector<ItemSubClass> > m;
public:
	void load() VISIBLE;

	const vector<ItemSubClass>& operator[](int i) const { return m[i]; }
	size_t size() const { return m.size(); }
};

extern ItemSubClasses gItemSubClasses VISIBLE;

#endif	//ITEM_SUB_CLASS_H
