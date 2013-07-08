#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcSkillLine.h"
#include "dbcSkillLineCategory.h"
#include "icon.h"
#include "dbcSpellIcon.h"
#include "itrPair.h"
#include "wowVersion.h"

class skillsChtml : public PageContext {
public:
	skillsChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);
	virtual ~skillsChtml();
};
