#include "npc.chtml.h"
#include "db_creature_template.h"
#include "db_spawn.h"
#include "questrelation.h"
#include "comments.h"
#include "item_shared.h"
#include "db_npc_vendor.h"
#include "db_item.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

static Tab* sells(int npcId);
//static Tab* teaches(int npcId);

void npcChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gNpcs.load();
	gCreatureSpawns.load();
	gFactions.load();
	gFactionTemplates.load();
	gNpcVendors.load();
	//gNpcTrainers.load();
	gItems.load();
	spawnPointsPrepare();

	int id = toInt(urlPart);
	a = gNpcs.find(id);
	if(a) {
		mTitle = a->name.c_str();

		mTabs.push_back(getQuestRelations("startQuest", "Starts quest", gCreatureQuestGivers, id));
		mTabs.push_back(getQuestRelations("endQuest", "Ends quest", gCreatureQuestFinishers, id));
		mTabs.push_back(sells(id));
		//mTabs.push_back(teaches(id));
		mTabs.push_back(getComments("npc", id));

		mSpawnPointsChtml.addSpawns(gCreatureSpawns.findId(id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

void npcChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

static Tab* sells(int npcId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "sells";
	t.title = "Sells";
	itemColumns(t);
	t.columns.push_back(Column(STOCK, "Stock"));
	auto res = gNpcVendors.findEntry(npcId);
	for(; res.first != res.second; ++res.first) {
		const NpcVendor& nv(*res.first->second);
		Row r;
		itemRow(r, gItems[nv.item]);
		if(nv.maxcount == 0)
			r[STOCK] = "∞";
		else
			r[STOCK] = toString(nv.maxcount);
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}
