#include "db_spawn.h"
#include "db_quest.h"

class QuestPointListener {
public:
	virtual void questGivers(Spawns::IdPair) = 0;
	virtual void questFinishers(Spawns::IdPair) = 0;
	virtual void questObjectives(Spawns::IdPair) = 0;
};

void getQuestLocations(const Quest&, QuestPointListener&) VISIBLE;
