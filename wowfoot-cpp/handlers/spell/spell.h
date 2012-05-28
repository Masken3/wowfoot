#include "pageContext.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "db_item.h"
#include "dbcSpell.h"
#include "dbcSpellIcon.h"
#include "icon.h"

class spellChtml : public tabsChtml, public PageContext {
public:
	spellChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* mTitle;
	const Spell* a;
};
