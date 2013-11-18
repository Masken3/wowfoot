#include "questrelation.h"
#include "tabTable.h"
#include "db_quest.h"
#include "chtmlUtil.h"
#include "util/criticalSection.h"
#include "db_spawn.h"
#include "spawnPoints.h"
#include "questTable.h"
#include "util/stl_map_insert.h"
#include <stdio.h>

enum QrRowId {
	NAME = ENTRY+1,
};

static CriticalSectionLoadGuard sCS;

Tab* getQuestRelations(const char* tabId, const char* title, QuestRelations& set, int id) {
	set.load();
	gQuests.load();

	tabTableChtml& t = *new tabTableChtml();
	t.id = tabId;
	t.title = title;
	questColumns(t);
	auto p = set.findId(id);
	for(; p.first != p.second; ++p.first) {
		const QuestRelation& qr(*p.first->second);
		const Quest* q = gQuests.find(qr.quest);
		Row r;
		if(q) {
			questRow(r, *q);
		} else {
			r[ENTRY] = toString(qr.quest);
			r[NAME] = "Unknown quest "+r[ENTRY];
		}
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static void initZoneQuestGiverSpawns(Spawns::IntMap& zqgs, Spawns& spawns, QuestRelations& qgs) {
	spawnPointsPrepare();
	spawns.load();
	qgs.load();
	printf("initZoneQuestGiverSpawns(%s)\n", spawns.name);
	for(auto itr = qgs.begin(); itr != qgs.end(); ++itr) {
		// assuming that quest givers only spawn once.
		// if they spawn more, we'll see duplicates.
		const QuestRelation& qr(*itr);
		for(auto sp = spawns.findId(qr.id); sp.first != sp.second; ++sp.first) {
			const Spawn& s(*sp.first->second);
			int zoneId = zoneFromCoords(s.map, s.position_x, s.position_y);
			insert(zqgs, zoneId, &s);
		}
	}
}

static Spawns::IntMap sCreatureZoneQuestGiverSpawns;
static Spawns::IntMap sObjectZoneQuestGiverSpawns;

static void init() {
	LOCK_AND_LOAD;
	initZoneQuestGiverSpawns(sCreatureZoneQuestGiverSpawns, gCreatureSpawns, gCreatureQuestGivers);
	initZoneQuestGiverSpawns(sObjectZoneQuestGiverSpawns, gGameobjectSpawns, gObjectQuestGivers);
}

Spawns::IntPair creatureZoneQuestGiverSpawns(int zoneId) {
	init();
	return sCreatureZoneQuestGiverSpawns.equal_range(zoneId);
}

Spawns::IntPair objectZoneQuestGiverSpawns(int zoneId) {
	init();
	return sObjectZoneQuestGiverSpawns.equal_range(zoneId);
}
