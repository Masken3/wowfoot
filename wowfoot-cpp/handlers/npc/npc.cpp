#include "npc.chtml.h"
#include "db_creature_template.h"
#include "db_spawn.h"
#include "questrelation.h"
#include "comments.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

void npcChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gNpcs.load();
	gCreatureSpawns.load();
	gFactions.load();
	gFactionTemplates.load();

	int id = toInt(urlPart);
	a = gNpcs.find(id);
	if(a) {
		mTitle = a->name.c_str();

		mTabs.push_back(getQuestRelations("Starts quest", gCreatureQuestGivers, id));
		mTabs.push_back(getQuestRelations("Ends quest", gCreatureQuestFinishers, id));
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
