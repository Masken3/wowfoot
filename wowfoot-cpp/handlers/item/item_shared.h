#ifndef ITEM_SHARED_H
#define ITEM_SHARED_H

#include "chtmlUtil.h"
#include "tabTable.h"

#include "wowVersion.h"
#if (CONFIG_WOW_VERSION > 30000)
#include "dbcTotemCategory.h"
#define HAVE_EXTENDED_COST 1
#define HAVE_TOTEM_CATEGORY 1
#else
#define HAVE_EXTENDED_COST 0
#define HAVE_TOTEM_CATEGORY 0
#endif

#include <ostream>

class Item;
class Row;
class Loot;
class Npc;

struct Quality {
	const char* color;
	const char* name;
};

void streamAllCostHtml(std::ostream&, const Item&) VISIBLE;
void streamItemClassHtml(std::ostream&, const Item&) VISIBLE;

// has "Heroic" prefix.
template<> void streamName(ostream& os, const Item& t) VISIBLE;

const Quality& ITEM_QUALITY(int id) VISIBLE;

void addItem(tabTableChtml& t, const Item& i) VISIBLE;
void itemColumns(tabTableChtml& t) VISIBLE;
void lootColumns(tabTableChtml& t) VISIBLE;
void lootRow(Row& r, const Loot& loot) VISIBLE;
void itemRow(Row& r, const Item& i) VISIBLE;
void npcColumns(tabTableChtml&) VISIBLE;
void npcRow(Row&, const Npc&) VISIBLE;

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
