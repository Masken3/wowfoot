#include "db_spawn.h"
#include "db_quest.h"
#include "dbcAreaTrigger.h"

class QuestPointListener {
public:
	virtual void questGivers(Spawns::IntPair) = 0;
	virtual void questFinishers(Spawns::IntPair) = 0;
	virtual void questObjectives(Spawns::IntPair) = 0;
	virtual void questAreaObjective(const AreaTrigger&) = 0;
};

void getQuestLocations(const Quest&, QuestPointListener&) VISIBLE;
