#ifndef DB_ITEM_H
#define DB_ITEM_H

#include "ConstMap.h"
#include "dllHelpers.h"
#include "db_item.struct.h"

class Items : public ConstMap<int, Item> {
public:
	void load() VISIBLE;
};

extern Items gItems VISIBLE;

#endif	//DB_ITEM_H
