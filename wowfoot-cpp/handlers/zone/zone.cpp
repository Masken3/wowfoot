#include "zone.chtml.h"
#include "questrelation.h"
#include "comments.h"
#include "tabTable.h"
#include "questTable.h"
#include "db_quest.h"
#include "../itemShared/item_shared.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

void zoneChtml::getResponse2(const char* u, DllResponseData* drd, ostream& os) {
	gWorldMapAreas.load();
	gAreaTable.load();
	gQuests.load();

	urlPart = u;
	mId = toInt(urlPart);
	mWMA = gWorldMapAreas.find(mId);
	mAT = gAreaTable.find(mId);
	//if(mWMA) {
		if(mAT) {
			mTitle = mAT->name;

			mSpawnPointsChtml.addSpawns(creatureZoneQuestGiverSpawns(mId), spawnPointsChtml::eYellow, "npc");
			mSpawnPointsChtml.addSpawns(objectZoneQuestGiverSpawns(mId), spawnPointsChtml::eYellow, "object");
			//mTabs.push_back(getZoneQuestGivers(mId));
		} else if(mWMA) {
			mTitle = mWMA->name;
	} else {
		mTitle = urlPart;
	}
	mTabs.push_back(getQuestsTab("quests", "Quests", mId, &Quests::findZoneOrSort));
	mTabs.push_back(getComments("zone", mId));
}

void zoneChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}
