#include "search.chtml.h"
#include "dllInterface.h"
#include "tabTable.h"
#include "dbcSpell.h"
#include "db_item.h"
#include "db_creature_template.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "win32.h"

using namespace std;

enum TableRowId {
	NAME = ENTRY+1,
};
#define MAX_COUNT 100

void searchChtml::getResponse2(const char* u, DllResponseData* drd, ostream& os) {
	gWorldMapAreas.load();
	gAreaTable.load();
	gSpells.load();
	gItems.load();
	gNpcs.load();

	urlPart = u;

	string searchString = toupper(urlPart);

	{
	tabTableChtml* tp = new tabTableChtml;
	tabTableChtml& t(*tp);
	t.id = "zone";
	t.title = "Zones";
	t.columns.push_back(Column(NAME, "Name", ENTRY, "zone"));

	for(AreaTable::citr itr = gAreaTable.begin(); itr != gAreaTable.end(); ++itr) {
		const Area& a(itr->second);
		if(a.parent == 0 &&
			strcasestr(a.name, urlPart))
		{
			Row r;
			r[ENTRY] = toString(itr->first);
			r[NAME] = a.name;
			t.array.push_back(r);
		}
	}
	t.count = t.array.size();
	mTabs.push_back(tp);
	}
	{
		tabTableChtml* tp = new tabTableChtml;
		tabTableChtml& t(*tp);
		t.id = "spell";
		t.title = "Spells";
		t.columns.push_back(Column(NAME, "Name", ENTRY, "spell"));
		for(Spells::citr itr = gSpells.begin();
			itr != gSpells.end() && t.array.size() < MAX_COUNT;
			++itr)
		{
			const Spell& s(itr->second);
			if(strcasestr(s.name, urlPart))
			{
				Row r;
				r[ENTRY] = toString(itr->first);
				r[NAME] = s.name;
				t.array.push_back(r);
			}
		}
		t.count = t.array.size();
		mTabs.push_back(tp);
	}
	{
		tabTableChtml* tp = new tabTableChtml;
		tabTableChtml& t(*tp);
		t.id = "item";
		t.title = "Items";
		t.columns.push_back(Column(NAME, "Name", ENTRY, "item"));
		for(Items::citr itr = gItems.begin();
			itr != gItems.end() && t.array.size() < MAX_COUNT;
			++itr)
		{
			const Item& s(itr->second);
			if(strcasestr(s.name.c_str(), urlPart))
			{
				Row r;
				r[ENTRY] = toString(itr->first);
				r[NAME] = s.name;
				t.array.push_back(r);
			}
		}
		t.count = t.array.size();
		mTabs.push_back(tp);
	}
	{
		tabTableChtml* tp = new tabTableChtml;
		tabTableChtml& t(*tp);
		t.id = "npc";
		t.title = "NPCs";
		t.columns.push_back(Column(NAME, "Name", ENTRY, "npc"));
		for(Npcs::citr itr = gNpcs.begin();
			itr != gNpcs.end() && t.array.size() < MAX_COUNT;
			++itr)
		{
			const Npc& s(itr->second);
			if(strcasestr(s.name.c_str(), urlPart))
			{
				Row r;
				r[ENTRY] = toString(itr->first);
				r[NAME] = s.name;
				t.array.push_back(r);
			}
		}
		t.count = t.array.size();
		mTabs.push_back(tp);
	}

	drd->code = run(os);
}
