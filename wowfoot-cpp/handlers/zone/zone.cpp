#include "zone.chtml.h"
#include "questrelation.h"
#include "comments.h"
#include "tabTable.h"
#include "db_quest.h"
#include "../item/item_shared.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

static Tab* getZoneQuests(int zoneId);

void zoneChtml::getResponse2(const char* u, DllResponseData* drd, ostream& os) {
	gWorldMapAreas.load();
	gAreaTable.load();
	gQuests.load();

	urlPart = u;
	mId = toInt(urlPart);
	mWMA = gWorldMapAreas.find(mId);
	mAT = gAreaTable.find(mId);
	if(mWMA) {
		if(mAT) {
			mTitle = mAT->name;

			mSpawnPointsChtml.addSpawns(creatureZoneQuestGiverSpawns(mId));
			mSpawnPointsChtml.addSpawns(objectZoneQuestGiverSpawns(mId));
			//mTabs.push_back(getZoneQuestGivers(mId));
		} else {
			mTitle = mWMA->name;
		}
	} else {
		mTitle = urlPart;
	}
	mTabs.push_back(getZoneQuests(mId));
	mTabs.push_back(getComments("zone", mId));
}

void zoneChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

static Tab* getZoneQuests(int zoneId) {
	tabTableChtml& t = *new tabTableChtml();
	t.id = "quests";
	t.title = "Quests";
	t.columns.push_back(Column(NAME, "Title", ENTRY, "quest"));
	auto res = gQuests.findZoneOrSort(zoneId);
	for(; res.first != res.second; ++res.first) {
		const Quest& q(*res.first->second);
		Row r;
		r[ENTRY] = toString(q.id);
		r[NAME] = q.title;
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}
