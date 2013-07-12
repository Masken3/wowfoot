#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcSpell.h"
#include "itrPair.h"
#include "icon.h"
#include "util/arraySize.h"
#include "db_item.h"
#include "dbcItemDisplayInfo.h"
#include "SkillLineAbility.index.h"

class skillChtml : public PageContext {
public:
	skillChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
	void httpArgument(const char* key, const char* value);
	virtual ~skillChtml();
private:
	ItrPair<Spell>* mPair;
	string mTitle;
};
