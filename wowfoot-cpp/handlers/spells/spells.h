#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcSpell.h"
#include "dbcSpellIcon.h"
#include "itrPair.h"
#include "icon.h"
#include "util/arraySize.h"
#include "db_item.h"
#include "item_shared.h"
#include "dbcItemDisplayInfo.h"
#include "SkillLineAbility.index.h"
#include "util/stl_map_insert.h"
#include <map>

class spellsChtml : public tabsChtml, public PageContext {
public:
	spellsChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
	void httpArgument(const char* key, const char* value);
	virtual ~spellsChtml();
private:
	string mTitle;
};
