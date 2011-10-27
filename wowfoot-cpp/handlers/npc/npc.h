#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "dbcWorldMapArea.h"
#include "dbcArea.h"
#include "tabs.h"
#include "db_creature_template.h"

class npcChtml : public tabsChtml {
public:
	int run(ostream& stream);
	const char* mTitle;
	string mMapName;
	const Npc* a;
};
