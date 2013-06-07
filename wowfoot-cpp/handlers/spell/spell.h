#include "pageContext.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "db_item.h"
#include "dbcSpell.h"
#include "dbcSpellIcon.h"
#include "dbcSpellMechanic.h"
#include "icon.h"
#include "SkillLineAbility.index.h"
#include "spellStrings.h"

class spellChtml : public tabsChtml, public PageContext {
public:
	spellChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	void streamPlainMembers(ostream&);
	void streamEffects(ostream&);

	const char* mTitle;
	const Spell* a;
};
