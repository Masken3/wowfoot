#ifndef ITEM_EXTENDED_COST_STRUCT_H
#define ITEM_EXTENDED_COST_STRUCT_H

struct ItemExtendedCost {
	struct ReqItem {
		int id, count;
	};
	int honorPoints;
	int arenaPoints;
	int arenaSlot;
	int arenaRating;
	ReqItem item[5];
};

#endif	//ITEM_EXTENDED_COST_STRUCT_H
