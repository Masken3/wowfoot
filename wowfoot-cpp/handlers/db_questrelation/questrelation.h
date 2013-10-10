#include "db_questrelation.h"
#include "tabs.h"
#include "db_spawn.h"

Tab* getQuestRelations(const char* tabId, const char* title, QuestRelations&, int id) VISIBLE;

Spawns::IntPair creatureZoneQuestGiverSpawns(int zoneId) VISIBLE;
Spawns::IntPair objectZoneQuestGiverSpawns(int zoneId) VISIBLE;
