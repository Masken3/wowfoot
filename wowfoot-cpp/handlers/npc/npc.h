#include "pageContext.h"
#include "tabs.h"
#include "db_creature_template.h"
#include "spawnPoints.h"
#include "dbcFaction.h"
#include "dbcFactionTemplate.h"

class npcChtml : public tabsChtml, public PageContext {
public:
	npcChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	string mMapName;
	const Npc* a;
	const Faction* mFaction;
	int mFactionId;
	spawnPointsChtml mSpawnPointsChtml;
};
