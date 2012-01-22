#ifndef ITEM_SHARED_H
#define ITEM_SHARED_H

#include "db_item.h"
#include "tabTable.h"
#include "db_loot_template.h"
#include <ostream>

void streamAllCostHtml(std::ostream&, const Item&) VISIBLE;
void streamItemClassHtml(std::ostream&, const Item&) VISIBLE;
void addItem(tabTableChtml& t, const Item& i) VISIBLE;
void itemColumns(tabTableChtml& t) VISIBLE;
void lootColumns(tabTableChtml& t) VISIBLE;
void lootRow(Row& r, const Loot& loot) VISIBLE;
void itemRow(Row& r, const Item& i) VISIBLE;

enum TableRowId {
	NAME = ENTRY+1,
	RESTRICTIONS,
	SOURCE,
	COST,
	LOCATION,
	ZONE,
	STOCK,
	ILEVEL,
	CLEVEL,
	SIDE,
	SLOT,
	TYPE,
	CHANCE,
	MIN_COUNT,
	MAX_COUNT,
	SPAWN_COUNT,
	UTILITY,
};

#endif	//ITEM_SHARED_H
