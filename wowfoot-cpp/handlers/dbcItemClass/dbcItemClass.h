#ifndef ITEM_CLASS_H
#define ITEM_CLASS_H

#include "dllHelpers.h"
#include <vector>

using namespace std;

struct ItemClass {
	const char* name;
};

class ItemClasses {
private:
	vector<ItemClass> m;
public:
	void load() VISIBLE;

	const ItemClass& operator[](int i) const { return m[i]; }
	size_t size() const { return m.size(); }
};

extern ItemClasses gItemClasses VISIBLE;

#endif	//ITEM_CLASS_H
