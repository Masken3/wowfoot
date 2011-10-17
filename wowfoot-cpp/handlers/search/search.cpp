#include "search.chtml.h"
#include "dllInterface.h"
#include "tabTables.h"
#include "Spell.h"
#include "db_item.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "win32.h"

using namespace std;

enum TableId {
	ZONE,
	SPELL,
	ITEM,
};
enum TableRowId {
	NAME = ENTRY+1,
};
#define MAX_COUNT 100

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	gWorldMapAreas.load();
	gAreaTable.load();
	gSpells.load();
	gItems.load();

	searchChtml context;
	context.urlPart = urlPart;

	string searchString = toupper(urlPart);

	{
	Table t;
	t.id = ZONE;
	t.title = "Zones";
	Column c = { NAME, "Name", false, true, ENTRY, "zone" };
	t.columns.push_back(c);

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
	context.mTables.push_back(t);
	}
	{
		Table t;
		t.id = SPELL;
		t.title = "Spells";
		Column c = { NAME, "Name", false, true, ENTRY, "spell" };
		t.columns.push_back(c);
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
		context.mTables.push_back(t);
	}
	{
		Table t;
		t.id = ITEM;
		t.title = "Items";
		Column c = { NAME, "Name", false, true, ENTRY, "item" };
		t.columns.push_back(c);
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
		context.mTables.push_back(t);
	}

	getResponse(drd, context);
}
