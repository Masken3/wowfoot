#define __STDC_FORMAT_MACROS
#include "wowVersion.h"
#include "quest.h"
#include "comments.h"
#include "db_creature_template.h"
#include "money.h"
#include "dbcSpell.h"
#include "db_item.h"
#include "db_loot_template.h"
#if CONFIG_WOW_VERSION > 30000
#include "dbcQuestFactionReward.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "win32.h"

using namespace std;

void questChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gMaps.load();
	gAreaQuestObjectives.load();
	gAreaTriggers.load();
	gCreatureQuestGivers.load();
	gObjectQuestGivers.load();
	printf("Loading finishers...\n");
	gCreatureQuestFinishers.load();
	gObjectQuestFinishers.load();
	printf("Finishers loaded.\n");
	gFactions.load();
#if CONFIG_WOW_VERSION > 30000
	gQuestFactionRewards.load();
#endif
	gNpcs.load();
	gItems.load();
	gSpells.load();
	gQuests.load();
	gObjects.load();
	spawnPointsPrepare();
	gCreatureLoots.load();
	gPickpocketingLoots.load();
	gGameobjectLoots.load();

	int id = toInt(urlPart);
	a = gQuests.find(id);
	if(a) {
		mTitle = a->title;

		// todo: fix this for version 1.12.
		int rewMoney = a->rewardMoneyMaxLevel;
		if(a->level >= 65)
			mRewXp = rewMoney / 6;
		else if(a->level == 64)
			mRewXp = int(rewMoney / 4.8);
		else if(a->level == 63)
			mRewXp = int(rewMoney / 3.6);
		else if(a->level == 62)
			mRewXp = int(rewMoney / 2.4);
		else if(a->level == 61)
			mRewXp = int(rewMoney / 1.2);
		else //if(a->level <= 60)
			mRewXp = int(rewMoney / 0.6);

		mTabs.push_back(getComments("quest", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
}

#if CONFIG_WOW_VERSION > 30000
int questChtml::rewRepValue(int index) {
	if(a->rewardFactionValueIdOverride[index] == 0) {
		int id = a->rewardFactionValueId[index];
		EASSERT(abs(id) <= 9);
		if(id >= 0)
			return gQuestFactionRewards[1].rep[id].value;
		else	// (id < 0)
			return gQuestFactionRewards[2].rep[-id].value;
	} else {
		return a->rewardFactionValueIdOverride[index] / 100;
	}
}
#endif

void questChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

void questChtml::streamEncodedObjectives(ostream& stream) {
	for(uint i=0; i<ARRAY_SIZE(a->objective); i++) {
		const Quest::Objective& o(a->objective[i]);
		bool empty = true;
		// objective.text is optional, used only for scripted objectives.
		if(!o.text.empty()) {
			empty = false;
			streamWowFormattedText(stream, o.text);
			stream << ". ";
		}
		if(o.reqSourceCount != 0) {
			empty = false;
			stream << "Source item: ";
			NAMELINK(gItems, o.reqSourceId);
			stream << " x"<<o.reqSourceCount<<"\n";
		}
		if(o.reqItemCount != 0) {
			empty = false;
			stream << "Item: ";
			NAMELINK(gItems, o.reqItemId);
			stream << " x"<<o.reqItemCount<<"\n";
		}
		if(o.reqSpellCast) {
			empty = false;
			stream << "Cast ";
			NAMELINK(gSpells, o.reqSpellCast);
			if(o.reqCreatureOrGOCount != 0) {
				stream << "on ";
			}
		} else if(o.reqCreatureOrGOId > 0) {
			empty = false;
			stream << "Kill ";
		} else if(o.reqCreatureOrGOId < 0) {
			empty = false;
			stream << "Use ";
		}
		if(o.reqCreatureOrGOId > 0) {
			stream << "creature ";
			NAMELINK(gNpcs, o.reqCreatureOrGOId);
			stream << " "<<o.reqCreatureOrGOCount<<" times.\n";
		} else if(o.reqCreatureOrGOId < 0) {
			stream << "object ";
			NAMELINK(gObjects, -o.reqCreatureOrGOId);
			stream << " "<<o.reqCreatureOrGOCount<<" times.\n";
		}
		if(!empty) {
			stream << "<br>\n";
		}
	}
	// todo: spawnPoints.
	for(auto p = gAreaQuestObjectives.findQuest(a->id); p.first != p.second; ++p.first) {
		const AreaTrigger& at(gAreaTriggers[p.first->second->id]);
		const Map& map(gMaps[at.map]);
		stream << "Visit POI "<<at.id<<": map "<<at.map<<" ("<<map.name<<"), "<<at.x<<" "<<at.y<<" "<<at.z<<", radius "<<at.radius<<"<br>\n";
	}

	getQuestLocations(*a, *this);
}

void questChtml::questGivers(Spawns::IntPair sp) {
	mSpawnPoints.addSpawns(sp, spawnPointsChtml::eBlue);
}
void questChtml::questFinishers(Spawns::IntPair sp) {
	mSpawnPoints.addSpawns(sp, spawnPointsChtml::eYellow);
}
void questChtml::questObjectives(Spawns::IntPair sp) {
	mSpawnPoints.addSpawns(sp, spawnPointsChtml::eRed);
}
void questChtml::questAreaObjective(const AreaTrigger& at) {
	mSpawnPoints.addSpawn(at.map, at.x, at.y, spawnPointsChtml::eGreen);
}
