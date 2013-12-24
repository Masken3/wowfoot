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
#include "dbcSpellDuration.h"
#include "dbcSpellRange.h"
#include "spellShared.h"
#include "db_gameobject_template.h"
#include "util/arraySize.h"

class spellChtml : public tabsChtml, public PageContext {
public:
	spellChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	void streamPlainMembers(ostream&);
	void streamEffects(ostream&);
	void streamPowerName(ostream&, int type);

	const char* mTitle;
	const Spell* a;
};

enum SpellRangeEnum {
	SPELL_RANGE_IDX_SELF_ONLY = 1,
	SPELL_RANGE_IDX_COMBAT = 2,
	SPELL_RANGE_IDX_ANYWHERE = 13,
};
