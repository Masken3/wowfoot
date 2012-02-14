#define __STDC_FORMAT_MACROS
#include "faction.chtml.h"
#include "comments.h"
#include "db_creature_template.h"
#include "db_quest.h"
#include "item_shared.h"
#include "FactionTemplate.index.h"

using namespace std;

static Tab* members(int factionId);

void factionChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gFactions.load();
	FactionTemplateIndex::load();
	gNpcs.load();
	gQuests.load();
	//gItems.load();

	int id = toInt(urlPart);
	a = gFactions.find(id);
	if(a) {
		mTitle = a->name;

		//members
		mTabs.push_back(members(id));
		//quests that modify reputation
		//mTabs.push_back(getQuests(id));
		//npcs that can be killed for reputation
		//mTabs.push_back(getEnemies(id));
		//items that require reputation to buy
		//mTabs.push_back(getItems(id));

		mTabs.push_back(getComments("faction", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}

	drd->code = run(os);
}

static Tab* members(int factionId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "members";
	t.title = "Members";
	npcColumns(t);
	t.columns.push_back(Column(SPAWN_COUNT, "Spawn count"));
	FactionTemplateIndex::FactionPair ftp = FactionTemplateIndex::findFaction(factionId);
	for(; ftp.first != ftp.second; ++ftp.first) {
		int f = ftp.first->second->first;
		//printf("FactionTemplate %i\n", f);
		Npcs::FactionPair p = gNpcs.findFaction(f);
		for(; p.first != p.second; ++p.first) {
			const Npc& npc(*p.first->second);
			Row r;
			npcRow(r, npc);
			r[SPAWN_COUNT] = toString(npc.spawnCount);
			t.array.push_back(r);
		}
	}
	t.count = t.array.size();
	return &t;
}
