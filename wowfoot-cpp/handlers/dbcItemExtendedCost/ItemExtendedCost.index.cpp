#define __STDC_FORMAT_MACROS
#include "ItemExtendedCost.index.h"
#include "dbcItemExtendedCost.h"
#include "db_item.h"
#include "db_npc_vendor.h"
#include "db_creature_template.h"
#include <inttypes.h>
#include <stdio.h>
#include <unordered_set>
#include "util/criticalSection.h"

using namespace std;
using namespace ItemExtendedCostIndex;

static CriticalSectionLoadGuard sCS;
static ItemCostMap sItemCostMap;
static ItemVendorCostMap sItemVendorCostMap;
static ItemVendorMap sItemVendorMap;
static ItemItemMap sItemItemMap;

namespace std {
template<class A, class B>
class hash<pair<A,B> > {
public:
	size_t operator()(const pair<A,B>& o) const {
		return hash<size_t>()(hash<A>()(o.first)) + hash<B>()(o.second);
	}
};
}

void ItemExtendedCostIndex::load() {
	LOCK_AND_LOAD;

	gItemExtendedCosts.load();
	gNpcs.load();
	gNpcVendors.load();
	gItems.load();

	unordered_set<pair<int,int> > itemNpcSet, itemItemSet;

	for(ItemExtendedCosts::citr itr = gItemExtendedCosts.begin();
		itr != gItemExtendedCosts.end(); ++itr)
	{
		int ecEntry = itr->first;
		const ItemExtendedCost& ec(itr->second);

		for(int i=0; i<5; i++) {
			int itemId = ec.item[i].id;
			if(itemId) {
				// sItemCostMap
				sItemCostMap.insert(pair<int, ItemExtendedCosts::citr>(itemId, itr));

				// sItemVendorCostMap
				NpcVendors::ExtendedCostPair ecp = gNpcVendors.findExtendedCost(ecEntry);
				for(; ecp.first != ecp.second; ++ecp.first) {
					const NpcVendor& nv(*ecp.first->second);
					sItemVendorCostMap.insert(pair<int, const NpcVendor*>(itemId, &nv));
					// sItemVendorMap
					// make sure we have unique item,npc pairs.
					if(itemNpcSet.insert(pair<int,int>(itemId, nv.entry)).second) {
						sItemVendorMap.insert(pair<int,const Npc*>(itemId, &gNpcs[nv.entry]));
					}
					// sItemItemMap
					// make sure we have unique item,item pairs.
					if(itemNpcSet.insert(pair<int,int>(itemId, nv.item)).second) {
						sItemItemMap.insert(pair<int,const Item*>(itemId, &gItems[nv.item]));
					}
				}
			}
		}
	}
	printf("ItemExtendedCostIndex: Loaded %"PRIuPTR" rows into %s\n",
		sItemCostMap.size(), "sItemCostMap");
	printf("ItemExtendedCostIndex: Loaded %"PRIuPTR" rows into %s\n",
		sItemVendorCostMap.size(), "sItemVendorCostMap");
	printf("ItemExtendedCostIndex: Loaded %"PRIuPTR" rows into %s\n",
		sItemVendorMap.size(), "sItemVendorMap");
	printf("ItemExtendedCostIndex: Loaded %"PRIuPTR" rows into %s\n",
		sItemItemMap.size(), "sItemItemMap");
}

ItemItemPair ItemExtendedCostIndex::findItemItem(int entry) {
	return sItemItemMap.equal_range(entry);
}
