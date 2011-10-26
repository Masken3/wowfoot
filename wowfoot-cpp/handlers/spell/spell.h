#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "tabTable.h"
#include "db_item.h"
#include "dbcSpell.h"

class spellChtml : public tabsChtml {
public:
	int run(ostream& stream);
	const char* mTitle;
	const Spell* a;
};
