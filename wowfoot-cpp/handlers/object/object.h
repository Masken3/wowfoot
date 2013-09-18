#include "pageContext.h"
#include "chtmlUtil.h"
#include "tabs.h"
#include "db_gameobject_template.h"
#include "spawnPoints.h"
#include "lockStrings.h"
#include "util/arraySize.h"
#include "dbcLock.h"
#include "db_item.h"

class objectChtml : public tabsChtml, public PageContext {
public:
	objectChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	void type(ostream&);

	const char* mTitle;
	string mMapName;
	const Object* a;
	spawnPointsChtml mSpawnPointsChtml;
};
