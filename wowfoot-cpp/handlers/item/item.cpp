#define __STDC_FORMAT_MACROS
#include "item.chtml.h"
#include "comments.h"
//#include "extendedCost.h"
#include "db_npc_vendor.h"
#include "db_creature_template.h"
#include "db_loot_template.h"
#include "dbcItemExtendedCost.h"
#include "chrClasses.h"
#include "chrRaces.h"
#include "ItemExtendedCost.index.h"
#include "money.h"
#include "util/exception.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <inttypes.h>

using namespace std;

static void createTabs(vector<Tab*>& tabs, const Item& a);
static Tab* soldBy(const Item& a);
static Tab* currencyFor(const Item& a);
static Tab* sharesModel(const Item& a);
static Tab* droppedBy(const Item& a);
static Tab* pickpocketedFrom(const Item& a);
static Tab* skinnedFrom(const Item& a);
static Tab* referenceLoot(const Item& a);

void init() __attribute((constructor));
void init() {
	printf("init item\n");
}

void fini() __attribute((destructor));
void fini() {
	printf("fini item\n");
}

void itemChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gReferenceLoots.load();
	gCreatureLoots.load();
	gPickpocketingLoots.load();
	gSkinningLoots.load();
	gNpcs.load();
	gNpcVendors.load();
	gItems.load();
	gTotemCategories.load();
	gItemExtendedCosts.load();
	ItemExtendedCostIndex::load();
	gSpells.load();

	int id = toInt(urlPart);
	a = gItems.find(id);
	if(a) {
		mTitle = a->name.c_str();
		mDps = 0.0;
		for(int i=0; i<1; i++) {
			float averageDmg = (a->dmg_min[i] + a->dmg_max[i]) / 2.0;
			if(fnz(averageDmg))
				mDps += averageDmg / (a->delay / 1000.0);
		}
		createTabs(mTabs, *a);
		//printf("sizeof(string): %" PRIuPTR "\n", sizeof(string));
		mTabs.push_back(getComments("item", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}

	drd->code = run(os);
}

static void createTabs(vector<Tab*>& tabs, const Item& a) {
	// Sold by (npc)
	tabs.push_back(soldBy(a));
	// Currency for (item)
	tabs.push_back(currencyFor(a));
	// Same model as (item)
	tabs.push_back(sharesModel(a));
	// Disenchants to (item)
	// Disenchanted from (item)
	// Dropped by (npc)
	tabs.push_back(droppedBy(a));
	// Contained in (gameobject)
	// Contained in (item)
	// Milled from (item)
	// Mills to (item)
	// Pickpocketed from (npc)
	tabs.push_back(pickpocketedFrom(a));
	// Prospected from (item)
	// Prospects (item)
	// Skinned from (npc)
	tabs.push_back(skinnedFrom(a));
	// Created by (spell)
	// Reagent for (spell)
	// Provided for quest
	// Required for quest
	// Quest reward choice
	// Quest reward
	// Reference loot (debug)
	tabs.push_back(referenceLoot(a));
}

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

static class streamUnlessEmptyClass {
private:
	const char* const mSep;
public:
	streamUnlessEmptyClass(const char* sep) : mSep(sep) {}
	friend ostream& operator<<(ostream& o, streamUnlessEmptyClass& s) {
		if(o.tellp() == 0)
			return o;
		o << s.mSep;
		return o;
	}
} costSep(", ");

static string costHtml(const Item& a, int extendedCostId) {
	ostringstream html;
	if(a.buyPrice == 0 && extendedCostId <= 0)
		return "No cost";
	if(a.buyPrice != 0 && (extendedCostId <= 0 || a.flagsExtra == 3))
		moneyHtml(html, a.buyPrice);
	if(extendedCostId <= 0)
		return html.str();

	const ItemExtendedCost* ecp = gItemExtendedCosts.find(extendedCostId);
	if(!ecp) {
		html << "Extended cost not found (id "<<extendedCostId<<")";
		return html.str();
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
			html << costSep << item.count<<"x <a href=\"item="<<item.id<<"\">"<<gItems[item.id].name<<"</a>";
		}
	}
	return html.str();
}

static void npcColumns(tabTableChtml& t) {
	t.columns.push_back(Column(NAME, "Name", ENTRY, "npc"));
	t.columns.push_back(Column(LOCATION, "Location", ZONE, "zone"));
}

static void npcRows(Row& r, const Npc& npc) {
	r[ENTRY] = toString(npc.entry);
	r[NAME] = npc.name;
	r[ZONE] = toString(-1);//mainZoneForNpc(nv.entry);
	r[LOCATION] = "not implemented";//gAreaTable[r[ZONE]].name;
}

static Tab* soldBy(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "soldBy";
	t.title = "Sold by";
	npcColumns(t);
	t.columns.push_back(Column(STOCK, "Stock"));
	t.columns.push_back(Column(COST, "Cost", Column::NoEscape));
	NpcVendors::ItemPair res = gNpcVendors.findItem(a.entry);
	for(; res.first != res.second; ++res.first) {
		const NpcVendor& nv(*res.first->second);
		Row r;
		npcRows(r, gNpcs[nv.entry]);
		//todo: add nv.incrtime, a.buyCount;
		if(nv.maxcount == 0)
			r[STOCK] = "∞";
		else
			r[STOCK] = toString(nv.maxcount);
		r[COST] = costHtml(a, nv.extendedCost);
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* npcLoot(const Item& a, const Loots& loots, const char* id, const char* title) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = id;
	t.title = title;
	npcColumns(t);
	t.columns.push_back(Column(CHANCE, "Chance %"));
	t.columns.push_back(Column(MIN_COUNT, "MinCount"));
	t.columns.push_back(Column(MAX_COUNT, "MaxCount"));
	t.columns.push_back(Column(SPAWN_COUNT, "Spawn count"));
	t.columns.push_back(Column(UTILITY, "Farming value (spawn * chance * (max+min)/2 / eliteFactor)"));
	Loots::ItemPair res = loots.findItem(a.entry);
	for(; res.first != res.second; ++res.first) {
		const Loot& loot(*res.first->second);
		Npcs::LootIdPair nres = gNpcs.findLootId(loot.entry);
		for(; nres.first != nres.second; ++nres.first) {
			const Npc& npc(*nres.first->second);
			Row r;
			npcRows(r, npc);
			r[CHANCE] = toString(loot.chance);
			r[MIN_COUNT] = toString(loot.minCountOrRef);
			r[MAX_COUNT] = toString(loot.maxCount);
			r[SPAWN_COUNT] = toString(npc.spawnCount);
			r[UTILITY] = toString(loot.chance * npc.spawnCount * (loot.minCountOrRef + loot.maxCount) / 200.0);
			t.array.push_back(r);
		}
	}
	t.count = t.array.size();
	return &t;
}

static Tab* droppedBy(const Item& a) {
	return npcLoot(a, gCreatureLoots, "droppedBy", "Dropped by");
}
static Tab* pickpocketedFrom(const Item& a) {
	return npcLoot(a, gPickpocketingLoots, "pickpocketedFrom", "Pickpocketed from");
}
static Tab* skinnedFrom(const Item& a) {
	return npcLoot(a, gSkinningLoots, "skinnedFrom", "Skinned from");
}

#if 0
static Tab* foundInObject(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "foundInObject";
	t.title = "Found in object";
	objectColumns(t);
	t.columns.push_back(Column(CHANCE, "Chance"));
	t.columns.push_back(Column(MIN_COUNT, "MinCount"));
	t.columns.push_back(Column(MAX_COUNT, "MaxCount"));
	t.columns.push_back(Column(SPAWN_COUNT, "Spawn count"));
	Loots::ItemPair res = gGameobjectLoots.findItem(a.entry);
	for(; res.first != res.second; ++res.first) {
		const Loot& loot(*res.first->second);
		Objects::LootIdPair nres = gObjects.findLootId(loot.entry);
		for(; nres.first != nres.second; ++nres.first) {
			const Object& o(*nres.first->second);
			Row r;
			objectRows(r, o);
			r[CHANCE] = toString(loot.chance);
			r[MIN_COUNT] = toString(loot.minCountOrRef);
			r[MAX_COUNT] = toString(loot.maxCount);
			r[SPAWN_COUNT] = toString(o.spawnCount);
			t.array.push_back(r);
		}
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}
#endif

static Tab* referenceLoot(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "referenceLoot";
	t.title = "Reference loot";
	t.columns.push_back(Column(CHANCE, "Chance"));
	t.columns.push_back(Column(MIN_COUNT, "MinCount"));
	t.columns.push_back(Column(MAX_COUNT, "MaxCount"));
	t.columns.push_back(Column(SPAWN_COUNT, "Other items count"));
	Loots::ItemPair res = gReferenceLoots.findItem(a.entry);
	for(; res.first != res.second; ++res.first) {
		const Loot& loot(*res.first->second);
		Row r;
		r[ENTRY] = toString(loot.entry);
		r[CHANCE] = toString(loot.chance);
		r[MIN_COUNT] = toString(loot.minCountOrRef);
		r[MAX_COUNT] = toString(loot.maxCount);
		size_t count = 0;
		Loots::EntryPair ep = gReferenceLoots.findEntry(loot.entry);
		for(; ep.first != ep.second; ++ep.first) {
			count++;
		}
		r[SPAWN_COUNT] = toString(count);
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static void itemColumns(tabTableChtml& t) {
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

static void addItem(tabTableChtml& t, const Item& i) {
	Row r;
	r[ENTRY] = toString(i.entry);

	string classes = chrClasses(i.allowableClass);
	string races = chrRaces(i.allowableRace);
	r[RESTRICTIONS] = classes;
	if(!classes.empty() && !races.empty())
		r[RESTRICTIONS] += " / ";
	r[RESTRICTIONS] += races;

	r[NAME] = i.name;
	r[ILEVEL] = toString(i.itemLevel);
	r[CLEVEL] = toString(i.requiredLevel);
	r[SLOT] = itemChtml::ITEM_EQUIP(i.inventoryType);
	r[TYPE] = itemChtml::ITEM_CLASS(i.class_) + string("/") +
		itemChtml::ITEM_SUBCLASS(i.class_, i.subclass);

	// check every vendor selling this item, to make sure costs are identical.
	NpcVendors::ItemPair nip = gNpcVendors.findItem(i.entry);
	int ec = -1;
	bool identicalCost = true;
	bool hasVendor = false;
	for(; nip.first != nip.second; ++nip.first) {
		hasVendor = true;
		const NpcVendor& nv(*nip.first->second);
		if(ec == -1)
			ec = nv.extendedCost;
		else if(ec != nv.extendedCost)
			identicalCost = false;
	}
	if(hasVendor) {
		if(identicalCost) {
			r[COST] = costHtml(i, ec);
		} else {
			r[COST] = "Differs between vendors";
		}
	}

	t.array.push_back(r);
}

static Tab* currencyFor(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "currencyFor";
	t.title = "Currency for";
	itemColumns(t);
	ItemExtendedCostIndex::ItemItemPair res = ItemExtendedCostIndex::findItemItem(a.entry);
	for(; res.first != res.second; ++res.first) {
		const Item& i(*res.first->second);
		addItem(t, i);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* sharesModel(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "sharesModel";
	//t.title = "Same model as";
	itemColumns(t);
	// insufficient; displayId controls more than model.
	t.title = "Same look as";
	Items::DisplayIdPair res = gItems.findDisplayId(a.displayId);
	for(; res.first != res.second; ++res.first) {
		const Item& i(*res.first->second);
		if(i.entry == a.entry)
			continue;
#if 1
		addItem(t, i);	// causes ConstMap[] exception
#else
		Row r;
		r[ENTRY] = toString(i.entry);
		r[NAME] = i.name;
		t.array.push_back(r);
#endif
	}
	t.count = t.array.size();
	return &t;
}


#define ITEM_RESISTANCES(m)\
	m(holy)\
	m(fire)\
	m(nature)\
	m(frost)\
	m(shadow)\
	m(arcane)\

#define ENUM_RESISTANCE(id) {&Item::id##_res, #id},

const itemChtml::Resistance itemChtml::mResistances[] = {
	ITEM_RESISTANCES(ENUM_RESISTANCE)
};
const int itemChtml::mnResistances = sizeof(itemChtml::mResistances) / sizeof(itemChtml::Resistance);

const char* itemChtml::ITEM_CLASS(int id) {
	switch(id) {
	case 0: return "Consumable";
	case 1: return "Container";
	case 2: return "Weapon";
	case 3: return "Gem";
	case 4: return "Armor";
	case 5: return "Reagent";
	case 6: return "Projectile";
	case 7: return "Trade Goods";
	case 8: return "Generic(OBSOLETE)";
	case 9: return "Recipe";
	case 10: return "Money";
	case 11: return "Quiver";
	case 12: return "Quest";
	case 13: return "Key";
	case 14: return "Permanent(OBSOLETE)";
	case 15: return "Miscellaneous";
	case 16: return "Glyph";
	default: return "bad ITEM_CLASS";
	}
}
const char* itemChtml::ITEM_SUBCLASS(int c, int sc) {
	switch(c) {
	case 0: switch(sc) {
		case 0: return "Consumable (Usability in combat is decided by the spell assigned.)";
		case 1: return "Potion";
		case 2: return "Elixir";
		case 3: return "Flask";
		case 4: return "Scroll";
		case 5: return "Food & Drink";
		case 6: return "Item Enhancement";
		case 7: return "Bandage";
		case 8: return "Other";
		default: return "bad ITEM_SUBCLASS";
		}
	case 1: switch(sc) {
		case 0: return "Bag";
		case 1: return "Soul Bag";
		case 2: return "Herb Bag";
		case 3: return "Enchanting Bag";
		case 4: return "Engineering Bag";
		case 5: return "Gem Bag";
		case 6: return "Mining Bag";
		case 7: return "Leatherworking Bag";
		case 8: return "Inscription Bag";
		default: return "bad ITEM_SUBCLASS";
		}
	case 2: switch(sc) {
		case 0: return "Axe 1H";
		case 1: return "Axe 2H";
		case 2: return "Bow";
		case 3: return "Gun";
		case 4: return "Mace 1H";
		case 5: return "Mace 2H";
		case 6: return "Polearm";
		case 7: return "Sword 1H";
		case 8: return "Sword 2H";
		case 9: return "Obsolete";
		case 10: return "Staff";
		case 11: return "Exotic";
		case 12: return "Exotic";
		case 13: return "Fist Weapon";
		case 14: return "Miscellaneous (Blacksmith Hammer, Mining Pick, etc.)";
		case 15: return "Dagger";
		case 16: return "Thrown";
		case 17: return "Spear";
		case 18: return "Crossbow";
		case 19: return "Wand";
		case 20: return "Fishing Pole";
		default: return "bad ITEM_SUBCLASS";
		}
	case 3: switch(sc) {
		case 0: return "Red";
		case 1: return "Blue";
		case 2: return "Yellow";
		case 3: return "Purple";
		case 4: return "Green";
		case 5: return "Orange";
		case 6: return "Meta";
		case 7: return "Simple";
		case 8: return "Prismatic";
		default: return "bad ITEM_SUBCLASS";
		}
	case 4: switch(sc) {
		case 0: return "Miscellaneous";
		case 1: return "Cloth";
		case 2: return "Leather";
		case 3: return "Mail";
		case 4: return "Plate";
		case 5: return "Buckler(OBSOLETE)";
		case 6: return "Shield";
		case 7: return "Libram";
		case 8: return "Idol";
		case 9: return "Totem";
		case 10: return "Sigil";
		default: return "bad ITEM_SUBCLASS";
		}
	case 5: switch(sc) {
		case 0: return "Reagent";
		default: return "bad ITEM_SUBCLASS";
		}
	case 6: switch(sc) {
		case 0: return "Wand(OBSOLETE)";
		case 1: return "Bolt(OBSOLETE)";
		case 2: return "Arrow";
		case 3: return "Bullet";
		case 4: return "Thrown(OBSOLETE)";
		default: return "bad ITEM_SUBCLASS";
		}
	case 7: switch(sc) {
		case 0: return "Trade Goods";
		case 1: return "Parts";
		case 2: return "Explosives";
		case 3: return "Devices";
		case 4: return "Jewelcrafting";
		case 5: return "Cloth";
		case 6: return "Leather";
		case 7: return "Metal & Stone";
		case 8: return "Meat";
		case 9: return "Herb";
		case 10: return "Elemental";
		case 11: return "Other";
		case 12: return "Enchanting";
		case 13: return "Materials";
		case 14: return "Armor Enchantment";
		case 15: return "Weapon Enchantment";
		default: return "bad ITEM_SUBCLASS";
		}
	case 8: switch(sc) {
		case 0: return "Generic(OBSOLETE)";
		default: return "bad ITEM_SUBCLASS";
		}
	case 9: switch(sc) {
		case 0: return "Book";
		case 1: return "Leatherworking";
		case 2: return "Tailoring";
		case 3: return "Engineering";
		case 4: return "Blacksmithing";
		case 5: return "Cooking";
		case 6: return "Alchemy";
		case 7: return "First Aid";
		case 8: return "Enchanting";
		case 9: return "Fishing";
		case 10: return "Jewelcrafting";
		default: return "bad ITEM_SUBCLASS";
		}
	case 10: switch(sc) {
		case 0: return "Money";
		default: return "bad ITEM_SUBCLASS";
		}
	case 11: switch(sc) {
		case 0: return "Quiver(OBSOLETE 0)";
		case 1: return "Quiver(OBSOLETE 1)";
		case 2: return "Quiver (Can hold arrows)";
		case 3: return "Ammo Pouch (Can hold bullets)";
		default: return "bad ITEM_SUBCLASS";
		}
	case 12: switch(sc) {
		case 0: return "Quest";
		default: return "bad ITEM_SUBCLASS";
		}
	case 13: switch(sc) {
		case 0: return "Key";
		case 1: return "Lockpick";
		default: return "bad ITEM_SUBCLASS";
		}
	case 14: switch(sc) {
		case 0: return "Permanent";
		default: return "bad ITEM_SUBCLASS";
		}
	case 15: switch(sc) {
		case 0: return "Junk";
		case 1: return "Reagent";
		case 2: return "Pet";
		case 3: return "Holiday";
		case 4: return "Other";
		case 5: return "Mount";
		default: return "bad ITEM_SUBCLASS";
		}
	case 16: switch(sc) {
		case 1: return "Warrior";
		case 2: return "Paladin";
		case 3: return "Hunter";
		case 4: return "Rogue";
		case 5: return "Priest";
		case 6: return "Death Knight";
		case 7: return "Shaman";
		case 8: return "Mage";
		case 9: return "Warlock";
		case 11: return "Druid";
		default: return "bad ITEM_SUBCLASS";
		}
	default: return "bad ITEM_CLASS";
	}
}
const char* itemChtml::ITEM_EQUIP(int id) {
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
const char* itemChtml::ITEM_BONDING(int id) {
	switch(id) {
	case 1: return "Bind on Pickup";
	case 2: return "Bind on Equip";
	case 3: return "Bind on Use";
	case 4: return "Quest item";
	case 5: return "Quest Item1";
	default: return "bad ITEM_BONDING";
	}
}
const char* itemChtml::ITEM_MATERIAL(int id) {
	switch(id) {
	case -1: return "Consumable";
	case 0: return "Not Defined";
	case 1: return "Metal";
	case 2: return "Wood";
	case 3: return "Liquid";
	case 4: return "Jewelry";
	case 5: return "Chain";
	case 6: return "Plate";
	case 7: return "Cloth";
	case 8: return "Leather";
	default: return "bad ITEM_MATERIAL";
	}
}
const char* itemChtml::ITEM_DAMAGE_TYPE(int id) {
	switch(id) {
	case 0: return "Physical";
	case 1: return "Holy";
	case 2: return "Fire";
	case 3: return "Nature";
	case 4: return "Frost";
	case 5: return "Shadow";
	case 6: return "Arcane";
	default: return "bad ITEM_DAMAGE_TYPE";
	}
}
const char* itemChtml::ITEM_STAT(int id) {
	switch(id) {
	case 0: return "Mana";
	case 1: return "Health";
	case 3: return "Agility";
	case 4: return "Strength";
	case 5: return "Intellect";
	case 6: return "Spirit";
	case 7: return "Stamina";
	case 12: return "Defense rating";
	case 13: return "Dodge rating";
	case 14: return "Parry rating";
	case 15: return "Block rating";
	case 16: return "Hit melee rating";
	case 17: return "Hit ranged rating";
	case 18: return "Hit spell rating";
	case 19: return "Crit melee rating";
	case 20: return "Crit ranged rating";
	case 21: return "Crit spell rating";
	case 22: return "Hit taken melee rating";
	case 23: return "Hit taken ranged rating";
	case 24: return "Hit taken spell rating";
	case 25: return "Crit taken melee rating";
	case 26: return "Crit taken ranged rating";
	case 27: return "Crit taken spell rating";
	case 28: return "Haste melee rating";
	case 29: return "Haste ranged rating";
	case 30: return "Haste spell rating";
	case 31: return "Hit rating";
	case 32: return "Crit rating";
	case 33: return "Hit taken rating";
	case 34: return "Crit taken rating";
	case 35: return "Resilience rating";
	case 36: return "Haste rating";
	case 37: return "Expertise rating";
	case 38: return "Attack power";
	case 39: return "Ranged attack power";
	case 40: return "Feral attack power (not used as of 3.3)";
	case 41: return "Spell healing done";
	case 42: return "Spell damage done";
	case 43: return "Mana regeneration";
	case 44: return "Armor penetration rating";
	case 45: return "Spell power";
	case 46: return "Health regen";
	case 47: return "Spell penetration";
	case 48: return "Block value";
	default: return "bad ITEM_STAT";
	}
}
const char* itemChtml::SPELLTRIGGER(int id) {
	switch(id) {
	case 0: return "Use";
	case 1: return "Equip";
	case 2: return "Chance on Hit";
	case 4: return "Soulstone";
	case 5: return "Use with no delay";
	case 6: return "Learn";
	default: return "bad SPELLTRIGGER";
	}
}

const itemChtml::Flag itemChtml::ITEM_FLAGS[] = {
	{1, "Soulbound"},
	{2, "Conjured"},
	{4, "Lootable"},
	{8, "Heroic"},
	{16, "Deprecated"},
	{32, "Totem"},
	{64, "Activatable"},
	{256, "Wrapper"},
	{1024, "Gift"},
	{2048, "Party loot"},
	{8192, "Charter (Arena or Guild)"},
	{32768, "PvP reward"},
	{524288, "Unique-equipped"},
	{4194304, "Throwable"},
	{8388608, "Special Use"},
	//{134221824, "Bind to Account"},
	{268435456, "Enchanting scroll"},
	{536870912, "Millable"},
	{2147483648u, "Bind on Pickup tradeable"},
};
const int itemChtml::nITEM_FLAGS = sizeof(ITEM_FLAGS) / sizeof(itemChtml::Flag);

const itemChtml::Flag itemChtml::ITEM_BAG_FAMILY[] = {
	{1, "Arrows"},
	{2, "Bullets"},
	{4, "Soul Shards"},
	{8, "Leatherworking Supplies"},
	{16, "Inscription Supplies"},
	{32, "Herbs"},
	{64, "Enchanting Supplies"},
	{128, "Engineering Supplies"},
	{256, "Keys"},
	{512, "Gems"},
	{1024, "Mining Supplies"},
	{2048, "Soulbound Equipment"},
	{4096, "Vanity Pets"},
	{8192, "Currency Tokens"},
	{16384, "Quest Items"},
};
const int itemChtml::nITEM_BAG_FAMILY = sizeof(ITEM_BAG_FAMILY) / sizeof(itemChtml::Flag);

const itemChtml::Quality& itemChtml::ITEM_QUALITY(int id) {
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
