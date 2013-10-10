#include "questPoints.h"
#include "db_questrelation.h"
#include "db_loot_template.h"
#include "dbcSpell.h"
#include "util/arraySize.h"
#include "win32.h"

typedef void (QuestPointListener::*QplFunc)(Spawns::IntPair);

static void getQuestLocationPair(Spawns::IntPair sp, QuestPointListener& l, QplFunc func) {
	(l.*func)(sp);
}
static void getQuestLocationByRelation(const Quest& q, QuestPointListener& l, QplFunc func,
	const QuestRelations& qrs, const Spawns& spawns)
{
	for(auto pair = qrs.findQuest(q.id); pair.first != pair.second; ++pair.first) {
		const QuestRelation& qr(*pair.first->second);
		getQuestLocationPair(spawns.findId(qr.id), l, func);
	}
}
static void getCreatureLocations(int id, QuestPointListener& l, QplFunc func) {
	getQuestLocationPair(gCreatureSpawns.findId(id), l, func);
}
static void getObjectLocations(int id, QuestPointListener& l, QplFunc func) {
	getQuestLocationPair(gGameobjectSpawns.findId(id), l, func);
}
static void getItemLocations(int id, QuestPointListener& l, QplFunc func) {
	for(auto p = gCreatureLoots.findItem(id); p.first != p.second; ++p.first) {
		getCreatureLocations(p.first->second->entry, l, func);
	}
	for(auto p = gPickpocketingLoots.findItem(id); p.first != p.second; ++p.first) {
		getCreatureLocations(p.first->second->entry, l, func);
	}
	for(auto p = gGameobjectLoots.findItem(id); p.first != p.second; ++p.first) {
		getObjectLocations(p.first->second->entry, l, func);
	}
}

void getQuestLocations(const Quest& q, QuestPointListener& l) {
	getQuestLocationByRelation(q, l, &QuestPointListener::questGivers, gCreatureQuestGivers, gCreatureSpawns);
	getQuestLocationByRelation(q, l, &QuestPointListener::questGivers, gObjectQuestGivers, gGameobjectSpawns);

	// todo: spawn points of sources of items that start quests.

	getQuestLocationByRelation(q, l, &QuestPointListener::questFinishers, gCreatureQuestFinishers, gCreatureSpawns);
	getQuestLocationByRelation(q, l, &QuestPointListener::questFinishers, gObjectQuestFinishers, gGameobjectSpawns);

	// spawn points of quest objectives.
	for(uint i=0; i<ARRAY_SIZE(q.objective); i++) {
		const Quest::Objective& o(q.objective[i]);
		if(o.reqSourceId) {
			getItemLocations(o.reqSourceId, l, &QuestPointListener::questObjectives);
		} else if(o.reqItemId) {
			getItemLocations(o.reqItemId, l, &QuestPointListener::questObjectives);
		} else if(o.reqCreatureOrGOId) {
			if(o.reqCreatureOrGOId > 0)
				getCreatureLocations(o.reqCreatureOrGOId, l, &QuestPointListener::questObjectives);
			else
				getObjectLocations(-o.reqCreatureOrGOId, l, &QuestPointListener::questObjectives);
		} else if(o.reqSpellCast) {
			// if spell requires a focus object, find that object.
			const Spell& s(gSpells[o.reqSpellCast]);
			if(s.RequiresSpellFocus)
				getObjectLocations(s.RequiresSpellFocus, l, &QuestPointListener::questObjectives);
		}
	}
	for(auto p = gAreaQuestObjectives.findQuest(q.id); p.first != p.second; ++p.first) {
		const AreaTrigger& at(gAreaTriggers[p.first->second->id]);
		l.questAreaObjective(at);
	}
}
