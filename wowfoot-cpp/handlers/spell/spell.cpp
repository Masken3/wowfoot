#define __STDC_FORMAT_MACROS
#include "spell.chtml.h"
#include "comments.h"
#include "db_creature_template.h"
#include "money.h"
#include "dbcSpell.h"
#include "db_item.h"

#if 0
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <inttypes.h>
#endif

using namespace std;

enum TableRowId {
	NAME = ENTRY+1,
};
#define MAX_COUNT 100

void spellChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gSpellIcons.load();
	gNpcs.load();
	gItems.load();
	gSpells.load();

	int id = toInt(urlPart);
	a = gSpells.find(id);
	if(a) {
		mTitle = a->name;

		// todo: Used by (item) (maybe gameobject too)
		{
			tabTableChtml* tp = new tabTableChtml;
			tabTableChtml& t(*tp);
			t.id = "item";
			t.title = "Used by";
			t.columns.push_back(Column(NAME, "Name", ENTRY, "item"));
			Items::SpellIdPair p = gItems.findSpellId(id);
			for(; p.first != p.second && t.array.size() < MAX_COUNT;
				++p.first)
			{
				const Item& s(*p.first->second);
				Row r;
				r[ENTRY] = toString(s.entry);
				r[NAME] = s.name;
				t.array.push_back(r);
			}
			t.count = t.array.size();
			mTabs.push_back(tp);
		}

		mTabs.push_back(getComments("spell", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}

	drd->code = run(os);
}
