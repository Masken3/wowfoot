#include "wowVersion.h"
#include "pageContext.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "db_item.h"
#include "db_quest.h"
#include "db_questrelation.h"
#include "db_creature_template.h"
#include "db_gameobject_template.h"
#include "dbcSpell.h"
#include "dbcAreaTrigger.h"
#include "dbcMap.h"
#include "dbcFaction.h"
#include "money.h"
#include "questPoints.h"
#include "spawnPoints.h"
#include "util/arraySize.h"
#include <stdio.h>

class questChtml : public tabsChtml, public PageContext, QuestPointListener {
public:
	questChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream&);
private:
	string mTitle;
	const Quest* a;
	int mRewXp;
	spawnPointsChtml mSpawnPoints;

	int rewRepValue(int index);
	void streamEncodedObjectives(ostream&);

	//QuestPointListener
	void questGivers(Spawns::IdPair);
	void questFinishers(Spawns::IdPair);
	void questObjectives(Spawns::IdPair);
	void questAreaObjective(const AreaTrigger&);
};
