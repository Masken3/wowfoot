#include "questrelation.h"
#include "tabTable.h"
#include "db_quest.h"
#include "chtmlUtil.h"
#include "util/criticalSection.h"
#include "db_spawn.h"
#include "spawnPoints.h"
#include "util/stl_map_insert.h"
#include <stdio.h>

enum QrRowId {
	NAME = ENTRY+1,
};

static CriticalSectionLoadGuard sCS;

Tab* getQuestRelations(const char* title, QuestRelations& set, int id) {
	set.load();
	gQuests.load();

	tabTableChtml& t = *new tabTableChtml();
	t.id = title;
	t.title = title;
	t.columns.push_back(Column(NAME, "Title", ENTRY, "quest"));
	auto p = set.findId(id);
	for(; p.first != p.second; ++p.first) {
		const QuestRelation& qr(*p.first->second);
		const Quest* q = gQuests.find(qr.quest);
		Row r;
		r[ENTRY] = toString(qr.quest);
		if(q)
			r[NAME] = q->title;
		else
			r[NAME] = "Unknown quest "+r[ENTRY];
		t.array.push_back(r);
	}
	t.count = t.array.size();
	return &t;
}

static void initZoneQuestGiverSpawns(Spawns::IdMap& zqgs, Spawns& spawns, QuestRelations& qgs) {
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
			insert(zqgs, Spawns::IdStruct {zoneId}, &s);
		}
	}
}

static Spawns::IdMap sCreatureZoneQuestGiverSpawns;
static Spawns::IdMap sObjectZoneQuestGiverSpawns;

static void init() {
	LOCK_AND_LOAD;
	initZoneQuestGiverSpawns(sCreatureZoneQuestGiverSpawns, gCreatureSpawns, gCreatureQuestGivers);
	initZoneQuestGiverSpawns(sObjectZoneQuestGiverSpawns, gGameobjectSpawns, gObjectQuestGivers);
}

Spawns::IdPair creatureZoneQuestGiverSpawns(int zoneId) {
	init();
	Spawns::IdStruct key = { zoneId };
	return sCreatureZoneQuestGiverSpawns.equal_range(key);
}

Spawns::IdPair objectZoneQuestGiverSpawns(int zoneId) {
	init();
	Spawns::IdStruct key = { zoneId };
	return sObjectZoneQuestGiverSpawns.equal_range(key);
}