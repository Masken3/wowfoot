#include "object.chtml.h"
#include "db_gameobject_template.h"
#include "db_spawn.h"
#include "comments.h"
#include "item_shared.h"
#include "db_loot_template.h"
#include "db_item.h"
#include "questrelation.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

static int GAMEOBJECT_TYPE_CHEST = 3;
static int GAMEOBJECT_TYPE_FISHINGHOLE = 25;

static Tab* contains(int entry);

void objectChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gObjects.load();
	gGameobjectSpawns.load();

	int id = toInt(urlPart);
	a = gObjects.find(id);
	if(a) {
		mTitle = a->name.c_str();
		mTabs.push_back(getQuestRelations("Starts quest", gObjectQuestGivers, id));
		mTabs.push_back(getQuestRelations("Ends quest", gObjectQuestFinishers, id));

		if(a->type == GAMEOBJECT_TYPE_CHEST ||
			a->type == GAMEOBJECT_TYPE_FISHINGHOLE)
		{
			mTabs.push_back(contains(a->data1));
		}
		mTabs.push_back(getComments("object", id));

		mSpawnPointsChtml.addSpawns(gGameobjectSpawns.findId(id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

void objectChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

static Tab* contains(int entry) {
	gGameobjectLoots.load();
	gItems.load();

	tabTableChtml& t = *new tabTableChtml();
	t.id = "contains";
	t.title = "Contains";
	itemColumns(t);
	lootColumns(t);
	Loots::EntryPair res = gGameobjectLoots.findEntry(entry);
	for(; res.first != res.second; ++res.first) {
		Row r;
		const Loot& l(*res.first->second);
		lootRow(r, l);
		const Item* pi = gItems.find(l.item);
		if(!pi) {
			r[NAME] = r[ENTRY] = toString(l.item);
		} else {
			itemRow(r, *pi);
		}
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static const char* sTypes[] = {
"Door",
"Button",
"Questgiver",
"Chest",
"Binder",
"Generic",
"Trap",
"Chair",
"Spell focus",
"Text",
"Goober",
"Transport",
"Area damage",
"Camera",
"Map object",
"Mo transport",
"Duel arbiter",
"Fishing node",
"Ritual",
"Mailbox",
"Auction house",
"Guard post",
"Spellcaster",
"Meeting stone",
"Flagstand",
"Fishing hole",
"Flagdrop",
"Mini game",
"Lottery kiosk",
"Capture point",
"Aura generator",
"Dungeon difficulty",
"Barber chair",
"Destructible building",
"Guild bank",
};
static const int snTypes = sizeof(sTypes) / sizeof(char*);

void objectChtml::type(ostream& o) {
	if(a->type < 0 || a->type >= snTypes) {
		o << "Invalid type " << a->type;
		return;
	}
	o << sTypes[a->type];
}
