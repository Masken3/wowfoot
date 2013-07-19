#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcSpell.h"
#include "itrPair.h"
#include "icon.h"
#include "util/arraySize.h"
#include "db_item.h"
#include "dbcItemDisplayInfo.h"
#include "SkillLineAbility.index.h"
#include "dbcSkillLine.h"
#include "skillShared.h"
#include "Lock.index.h"
#include "db_gameobject_template.h"
#include <map>
#include "util/stl_map_insert.h"

class skillChtml : public PageContext {
public:
	skillChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
	void httpArgument(const char* key, const char* value);
	virtual ~skillChtml();
private:
	const SkillLine* mSL;
	string mTitle;
};
