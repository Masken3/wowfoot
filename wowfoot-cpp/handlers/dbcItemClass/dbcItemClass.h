#ifndef ITEM_CLASS_H
#define ITEM_CLASS_H

#include "dllHelpers.h"
#include <vector>

using namespace std;

struct ItemClass {
	const char* name;
};

class ItemClasses : protected vector<ItemClass> {
private:
	typedef vector<ItemClass> super;
public:
	void load() VISIBLE;

	const ItemClass& operator[](int i) const { return super::operator[](i); }
	size_t size() const { return super::size(); }
};

extern ItemClasses gItemClasses VISIBLE;

#endif	//ITEM_CLASS_H
