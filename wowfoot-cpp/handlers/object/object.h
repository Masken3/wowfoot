#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "tabs.h"
#include "db_gameobject_template.h"
#include "spawnPoints.h"

class objectChtml : public tabsChtml, public PageContext {
public:
	int run(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	void type(ostream&);

	const char* mTitle;
	string mMapName;
	const Object* a;
	spawnPointsChtml mSpawnPointsChtml;
};
