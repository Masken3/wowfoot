#define __STDC_FORMAT_MACROS
#include "item.chtml.h"
#include "item_shared.h"
#include "comments.h"
#include "db_npc_vendor.h"
#include "db_gameobject_template.h"
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
#include "spellShared.h"
#include "Spell.index.h"
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
static Tab* questObjective(const Item& a);
static Tab* questObjectiveSource(const Item& a);
static Tab* questReward(const Item& a);
static Tab* questSourced(const Item& a);
static Tab* soldBy(const Item& a);
#if HAVE_EXTENDED_COST
static Tab* currencyFor(const Item& a);
#endif
static Tab* sharesModel(const Item& a);
static Tab* reagentFor(const Item& a);

void init() __attribute((constructor));
void init() {
	printf("init item\n");
}

void fini() __attribute((destructor));
void fini() {
	printf("fini item\n");
}

void itemChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gItemSets.load();
	gQuests.load();
	gSkillLines.load();
	gItemDisplayInfos.load();
	gReferenceLoots.load();
	gCreatureLoots.load();
	gPickpocketingLoots.load();
	gSkinningLoots.load();
	gDisenchantLoots.load();
	gItemLoots.load();
	gNpcs.load();
	gNpcVendors.load();
	gItems.load();
#if HAVE_TOTEM_CATEGORY
	gTotemCategories.load();
#endif
#if HAVE_EXTENDED_COST
	gItemExtendedCosts.load();
	ItemExtendedCostIndex::load();
#endif
	gSpells.load();
	gObjects.load();
	gGameobjectLoots.load();
	SpellIndex::load();

	string buffer;

	int id = toInt(urlPart);
	a = gItems.find(id);
	if(a) {
		if(a->flags & ITEM_FLAG_HEROIC) {
			buffer = "Heroic " + a->name;
			mTitle = buffer.c_str();
		} else {
			mTitle = a->name.c_str();
		}
		mDps = 0.0;
		for(int i=0; i<1; i++) {
			float averageDmg = (a->dmg_min[i] + a->dmg_max[i]) / 2.0;
			if(fnz(averageDmg))
				mDps += averageDmg / (a->delay / 1000.0);
		}
		createTabs(mTabs, *a);
		//printf("sizeof(string): %" PRIuPTR "\n", sizeof(string));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
	mTabs.push_back(getComments("item", id));
}

void itemChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

static void createTabs(vector<Tab*>& tabs, const Item& a) {
	// Provided for quest
	tabs.push_back(questSourced(a));
	// Required for quest
	tabs.push_back(questObjective(a));
	tabs.push_back(questObjectiveSource(a));
	// Quest reward choice
	// Quest reward
	tabs.push_back(questReward(a));
	// Sold by (npc)
	tabs.push_back(soldBy(a));
#if HAVE_EXTENDED_COST
	// Currency for (item)
	tabs.push_back(currencyFor(a));
#endif
	// Same model as (item)
	tabs.push_back(sharesModel(a));
	// Disenchants to (item)
	tabs.push_back(disenchantsTo(a));

	referentialLoots(tabs, a.entry);

	// Contains (items)
	tabs.push_back(contains(a.entry));
	// Mills to (item)
	// Prospects (item)
	// Created by (spell)
	// Reagent for (spell)
	tabs.push_back(reagentFor(a));
}

static Tab* questObjective(const Item& a) {
	return questObjectiveT(a.entry, "questObjective", "Quest objective",
		&Quests::findReqItemId, &Quest::Objective::reqItemId, &Quest::Objective::reqItemCount);
}

static Tab* questObjectiveSource(const Item& a) {
#if 0
	auto map = gQuests.getReqSourceIdMap();
	for(auto itr = map.begin(); itr != map.end(); ++itr) {
		printf("%i: %i (%s)\n", itr->first, itr->second->id, itr->second->title.c_str());
	}
#endif
	return questObjectiveT(a.entry, "questObjectiveSource", "Quest objective source",
		&Quests::findReqSourceId, &Quest::Objective::reqSourceId, &Quest::Objective::reqSourceCount);
}

static Tab* questReward(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "questReward";
	t.title = "Quest reward";
	questColumns(t);
	t.columns.push_back(Column(MAX_COUNT, "Count"));
	auto res = gQuests.findRewItemId(a.entry);
	for(; res.first != res.second; ++res.first) {
		const Quest& q(*res.first->second);
		Row r;
		questRow(r, q);
		for(size_t i=0; i<ARRAY_SIZE(q.rewardItemId); i++) {
			if(q.rewardItemId[i] == a.entry) {
				r[MAX_COUNT] = toString(q.rewardItemCount[i]);
			}
		}
		t.array.push_back(r);
	}
	res = gQuests.findRewChoiceItemId(a.entry);
	for(; res.first != res.second; ++res.first) {
		const Quest& q(*res.first->second);
		Row r;
		r[ENTRY] = toString(q.id);
		r[NAME] = q.title;
		for(size_t i=0; i<ARRAY_SIZE(q.rewardChoiceItemId); i++) {
			if(q.rewardChoiceItemId[i] == a.entry) {
				r[MAX_COUNT] = toString(q.rewardChoiceItemCount[i]);
			}
		}
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static Tab* questSourced(const Item& a) {
	return getQuestsTab("questSourced", "Given at quest start", a.entry, &Quests::findSrcItemId);
}

static Tab* soldBy(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "soldBy";
	t.title = "Sold by";
	npcColumns(t);
	t.columns.push_back(Column(STOCK, "Stock"));
	t.columns.push_back(Column(COST, "Cost", Column::NoEscape));
	NpcVendors::IntPair res = gNpcVendors.findItem(a.entry);
	for(; res.first != res.second; ++res.first) {
		const NpcVendor& nv(*res.first->second);
		Row r;
		npcRow(r, gNpcs[nv.entry]);
		//todo: add nv.incrtime, a.buyCount;
		if(nv.maxcount == 0)
			r[STOCK] = "∞";
		else
			r[STOCK] = toString(nv.maxcount);
		ostringstream oss;
		streamCostHtml(oss, a,
#if HAVE_EXTENDED_COST
			nv.extendedCost
#else
			-1
#endif
		);
		r[COST] = oss.str();
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

#if HAVE_EXTENDED_COST
static Tab* currencyFor(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "currencyFor";
	t.title = "Currency for";
	itemColumns(t);
	ItemExtendedCostIndex::IntPair res = ItemExtendedCostIndex::findItemItem(a.entry);
	for(; res.first != res.second; ++res.first) {
		const Item& i(*res.first->second);
		addItem(t, i);
	}
	t.count = t.array.size();
	return &t;
}
#endif

static Tab* sharesModel(const Item& a) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "sharesModel";
	//t.title = "Same model as";
	itemColumns(t);
	// insufficient; displayId controls more than model.
	t.title = "Same look as";
	Items::IntPair res = gItems.findDisplayId(a.displayId);
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

static Tab* reagentFor(const Item& a) {
	ItrPairImpl<Spell, SpellIndex::SpellItr> p(SpellIndex::findReagent(a.entry));
	return spellsTab("reagentFor", "Reagent for", p, true);
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
	{(int)2147483648u, "Bind on Pickup tradeable"},
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
