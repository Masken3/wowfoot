#define __STDC_FORMAT_MACROS
#include "item_shared.h"
#include "db_npc_vendor.h"
#include "db_item.h"
#include "db_loot_template.h"
#include "db_creature_template.h"
#include "dbcItemSubClass.h"
#include "dbcItemClass.h"
#include "chrClasses.h"
#include "chrRaces.h"
#if HAVE_EXTENDED_COST
#include "dbcItemExtendedCost.h"
#include "ItemExtendedCost.index.h"
#endif
#include "money.h"
#include "questObjectiveT.h"
#include "questTable.h"
#include "util/exception.h"
#include "util/arraySize.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <inttypes.h>

void streamAllCostHtml(std::ostream& o, const Item& i) {
	gNpcVendors.load();
	// check every vendor selling this item, to make sure costs are identical.
	NpcVendors::IntPair nip = gNpcVendors.findItem(i.entry);
	int ec = -1;
	bool identicalCost = true;
	bool hasVendor = false;
	for(; nip.first != nip.second; ++nip.first) {
		hasVendor = true;
#if HAVE_EXTENDED_COST
		const NpcVendor& nv(*nip.first->second);
		if(ec == -1)
			ec = nv.extendedCost;
		else if(ec != nv.extendedCost)
			identicalCost = false;
#endif
	}
	if(hasVendor) {
		if(identicalCost) {
			streamCostHtml(o, i, ec);
		} else {
			o << "Differs between vendors";
		}
	}
}

void streamItemClassHtml(std::ostream& o, const Item& i) {
	gItemClasses.load();
	gItemSubClasses.load();
	if((size_t)i.class_ >= gItemClasses.size()) {
		o << "Bad item.class (" << i.class_ << ")";
		return;
	}
	const ItemClass& c(gItemClasses[i.class_]);
	streamEscape(streamHtmlEscape, o, c.name);
	if((size_t)i.class_ >= gItemSubClasses.size()) {
		o << " / Bad item.class (" << i.class_ << ")";
		return;
	}
	auto isc = gItemSubClasses[i.class_];
	if(isc.size() > 1) {
		o << " / ";
		if((size_t)i.subclass >= isc.size()) {
			o << "Bad item.subclass (" << i.subclass << ")";
			return;
		}
		const ItemSubClass& sc(isc[i.subclass]);
		if(*sc.plural)
			streamEscape(streamHtmlEscape, o, sc.plural);
		else
			streamEscape(streamHtmlEscape, o, sc.name);
	}
}

template<> void streamName(ostream& os, const Item& t) {
	if(t.flags & ITEM_FLAG_HEROIC)
		os << "Heroic ";
	os << t.name;
}

const Quality& ITEM_QUALITY(int id) {
	switch(id) {
	case 0: { static const Quality q = { "9d9d9d", "Poor" }; return q; }
	case 1: { static const Quality q = { "ffffff", "Common" }; return q; }
	case 2: { static const Quality q = { "1eff00", "Uncommon" }; return q; }
	case 3: { static const Quality q = { "0080ff", "Rare" }; return q; }
	case 4: { static const Quality q = { "a335ee", "Epic" }; return q; }
	case 5: { static const Quality q = { "ff8000", "Legendary" }; return q; }
	case 6: { static const Quality q = { "ff0000", "Artifact" }; return q; }
	case 7: { static const Quality q = { "e6cc80", "Bind to Account" }; return q; }
	default: throw Exception("bad ITEM_QUALITY");
	}
}

void addItem(tabTableChtml& t, const Item& i) {
	Row r;
	itemRow(r, i);
	t.array.push_back(r);
}

void itemRow(Row& r, const Item& i) {
	ostringstream oss;
	r[ENTRY] = toString(i.entry);

	string classes = chrClasses(i.allowableClass);
	string races = chrRaces(i.allowableRace);
	r[RESTRICTIONS] = classes;
	if(!classes.empty() && !races.empty())
		r[RESTRICTIONS] += " / ";
	r[RESTRICTIONS] += races;

	if(i.flags & ITEM_FLAG_HEROIC)
		r[NAME] = string("Heroic ") + i.name;
	else
		r[NAME] = i.name;
	r[ILEVEL] = toString(i.itemLevel);
	r[CLEVEL] = toString(i.requiredLevel);
	r[SLOT] = ITEM_EQUIP(i.inventoryType);
	oss.str("");
	streamItemClassHtml(oss, i);
	r[TYPE] = oss.str();

	// check every vendor selling this item, to make sure costs are identical.
	oss.str("");
	streamAllCostHtml(oss, i);
	r[COST] = oss.str();
}

void itemColumns(tabTableChtml& t) {
	t.columns.push_back(Column(NAME, "Name", ENTRY, "item"));
	t.columns.push_back(Column(RESTRICTIONS, "Class/Race"));
	// name of and link to single vendor, or number of vendors.
	//t.columns.push_back(Column(VENDOR, "Vendor", Column::NoEscape));
	t.columns.push_back(Column(ILEVEL, "iLevel"));
	t.columns.push_back(Column(CLEVEL, "Req."));	//Required character level
	//t.columns.push_back(Column(SIDE, "Side"));	// Horde, Alliance, or none
	t.columns.push_back(Column(SLOT, "Slot"));
	t.columns.push_back(Column(TYPE, "Type"));
	t.columns.push_back(Column(SOURCE, "Source"));
	t.columns.push_back(Column(COST, "Cost", Column::NoEscape));
}

void lootColumns(tabTableChtml& t) {
	t.columns.push_back(Column(CHANCE, "Chance %"));
	t.columns.push_back(Column(MIN_COUNT, "MinCount"));
	t.columns.push_back(Column(MAX_COUNT, "MaxCount"));
}

void lootRow(Row& r, const Loot& loot) {
	r[CHANCE] = toString(loot.chance);
	r[MIN_COUNT] = toString(loot.minCountOrRef);
	r[MAX_COUNT] = toString(loot.maxCount);
}

void npcColumns(tabTableChtml& t) {
	t.columns.push_back(Column(NAME, "Name", ENTRY, "npc"));
	//t.columns.push_back(Column(LOCATION, "Location", ZONE, "zone"));
}

void npcRow(Row& r, const Npc& npc) {
	r[ENTRY] = toString(npc.entry);
	r[NAME] = npc.name;
	if(npc.subName.size() > 0)
		r[NAME] += " <"+npc.subName+">";
	r[ZONE] = toString(-1);//mainZoneForNpc(nv.entry);
	//r[LOCATION] = "not implemented";//gAreaTable[r[ZONE]].name;
}

class streamIfNonFirstClass {
private:
	const char* const mSep;
	bool mHit;
public:
	streamIfNonFirstClass(const char* sep) : mSep(sep) {}
	friend ostream& operator<<(ostream& o, streamIfNonFirstClass& s) {
		if(s.mHit)
			o << s.mSep;
		else
			s.mHit = true;
		return o;
	}
	void reset() { mHit = false; }
};

void streamCostHtml(ostream& html, const Item& a, int extendedCostId) {
	streamIfNonFirstClass costSep(", ");
#if HAVE_EXTENDED_COST
	gItemExtendedCosts.load();
#endif
	if(a.buyPrice == 0 && extendedCostId <= 0) {
		html << "No cost";
		return;
	}
#if HAVE_EXTENDED_COST
	if(a.buyPrice != 0 && (extendedCostId <= 0 || a.flagsExtra == 3))
#endif
	{
		html << "<!--"<<a.buyPrice<<"-->";	// improved sorting.
		moneyHtml(html, a.buyPrice);
	}
	if(extendedCostId <= 0)
		return;

#if HAVE_EXTENDED_COST
	const ItemExtendedCost* ecp = gItemExtendedCosts.find(extendedCostId);
	if(!ecp) {
		html << "Extended cost not found (id "<<extendedCostId<<")";
		return;
	}
	const ItemExtendedCost& ec(*ecp);
	if(ec.honorPoints != 0)
		html << costSep << ec.honorPoints<<" honor points";
	if(ec.arenaPoints != 0)
		html << costSep << ec.arenaPoints<<" arena points";
	if(ec.arenaRating != 0) {
		html << costSep << ec.arenaRating<<" ";
		switch(ec.arenaSlot) {
		case 0: html << "2v2"; break;
		case 1: html << "3v3/5v5"; break;
		case 2: html << "5v5"; break;
		default: html << "invalid arenaSlot ("<<ec.arenaSlot<<")";
		}
		html << " arena rating";
	}
	for(int i=0; i<5; i++) {
		ItemExtendedCost::ReqItem item(ec.item[i]);
		if(item.id != 0 || item.count != 0) {
			html << costSep << item.count<<"x ";
			streamNameLinkById(html, gItems, item.id);
		}
	}
#endif
}

const char* ITEM_EQUIP(int id) {
	switch(id) {
	case 0: return "";
	case 1: return "Head";
	case 2: return "Neck";
	case 3: return "Shoulder";
	case 4: return "Shirt";
	case 5: return "Chest";
	case 6: return "Waist";
	case 7: return "Legs";
	case 8: return "Feet";
	case 9: return "Wrists";
	case 10: return "Hands";
	case 11: return "Finger";
	case 12: return "Trinket";
	case 13: return "Weapon";
	case 14: return "Shield";
	case 15: return "Ranged";
	case 16: return "Back";
	case 17: return "Two-Hand";
	case 18: return "Bag";
	case 19: return "Tabard";
	case 20: return "Robe";
	case 21: return "Main hand";
	case 22: return "Off hand";
	case 23: return "Holdable (Tome)";
	case 24: return "Ammo";
	case 25: return "Thrown";
	case 26: return "Ranged (right-hand)";
	case 27: return "Quiver";
	case 28: return "Relic";
	default: return "bad ITEM_EQUIP";
	}
}

Tab* questObjectiveT(int entry, const char* id, const char* title,
	Quests::IntPair (Quests::*finder)(int)const, int Quest::Objective::*entryP, int Quest::Objective::*countP)
{
	tabTableChtml& t = *new tabTableChtml();
	t.id = id;
	t.title = title;
	questColumns(t);
	t.columns.push_back(Column(MAX_COUNT, "Count"));
	auto res = (gQuests.*finder)(entry);
	for(; res.first != res.second; ++res.first) {
		const Quest& q(*res.first->second);
		Row r;
		questRow(r, q);
		for(size_t i=0; i<ARRAY_SIZE(q.objective); i++) {
			const Quest::Objective& o(q.objective[i]);
			if(o.*entryP == entry) {
				r[MAX_COUNT] = toString(o.*countP);
			}
		}
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}
