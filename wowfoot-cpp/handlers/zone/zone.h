#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcWorldMapArea.h"
#include "dbcArea.h"
#include "mapSize.h"
#include "tabs.h"
#include "spawnPoints.h"

class zoneChtml : public tabsChtml, public PageContext {
public:
	zoneChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* urlPart;
	string mTitle;
	int mId;
	WorldMapAreas::ptr mWMA;
	AreaTable::ptr mAT;
	spawnPointsChtml mSpawnPointsChtml;
};
