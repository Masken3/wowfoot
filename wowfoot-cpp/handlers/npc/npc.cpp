#include "npc.chtml.h"
#include "db_creature_template.h"
#include "db_spawn.h"
#include "comments.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

void npcChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gNpcs.load();
	gCreatureSpawns.load();

	int id = toInt(urlPart);
	a = gNpcs.find(id);
	if(a) {
		mTitle = a->name.c_str();
		mTabs.push_back(getComments("npc", id));

		mSpawnPointsChtml.mSpawns = gCreatureSpawns.findId(id);
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
	drd->code = run(os);
}
