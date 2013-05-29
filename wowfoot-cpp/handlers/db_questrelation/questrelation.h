#include "db_questrelation.h"
#include "tabs.h"
#include "db_spawn.h"

Tab* getQuestRelations(const char* title, QuestRelations&, int id) VISIBLE;

Spawns::IdPair creatureZoneQuestGiverSpawns(int zoneId) VISIBLE;
Spawns::IdPair objectZoneQuestGiverSpawns(int zoneId) VISIBLE;
