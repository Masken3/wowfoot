#ifndef ITEM_EXTENDED_COST_INDEX_H
#define ITEM_EXTENDED_COST_INDEX_H

#include "ItemExtendedCost.h"
#include "dllHelpers.h"
#include "db_npc_vendor.struct.h"
#include "db_creature_template.struct.h"
#include "db_item.struct.h"
#include <unordered_map>

using namespace std;

namespace ItemExtendedCostIndex {
	void load() VISIBLE;

	// finds costs where the specified item is a part.
	typedef unordered_multimap<int, ItemExtendedCosts::citr> ItemCostMap;
	typedef ItemCostMap::const_iterator ItemCostItr;
	typedef pair<ItemCostItr, ItemCostItr> ItemCostPair;
	ItemCostPair findItemCost(int entry) VISIBLE;

	// finds vendor-item entries where the specified item is part of a cost.
	typedef unordered_multimap<int, const NpcVendor*> ItemVendorCostMap;
	typedef ItemVendorCostMap::const_iterator ItemVendorCostItr;
	typedef pair<ItemVendorCostItr, ItemVendorCostItr> ItemVendorCostPair;
	ItemVendorCostPair findItemVendorCost(int entry) VISIBLE;

	// finds vendors that accept the specified item as currency.
	typedef unordered_multimap<int, const Npc*> ItemVendorMap;
	typedef ItemVendorMap::const_iterator ItemVendorItr;
	typedef pair<ItemVendorItr, ItemVendorItr> ItemVendorPair;
	ItemVendorPair findItemVendor(int entry) VISIBLE;

	// finds items where the specified item is part of a cost.
	typedef unordered_multimap<int, const Item*> ItemItemMap;
	typedef ItemItemMap::const_iterator ItemItemItr;
	typedef pair<ItemItemItr, ItemItemItr> ItemItemPair;
	ItemItemPair findItemItem(int entry) VISIBLE;
};

#endif	//ITEM_EXTENDED_COST_INDEX_H
