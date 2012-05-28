#include "search.chtml.h"
#include "dllInterface.h"
#include "tabTable.h"
#include "dbcSpell.h"
#include "dbcAchievement.h"
#include "db_item.h"
#include "db_creature_template.h"
#include "db_gameobject_template.h"
#include "db_quest.h"
#include "dbcCharTitles.h"
#include "dbcItemSet.h"
#include "dbcFaction.h"

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

static const char* cstr(const char* s) { return s; }
static const char* cstr(const string& s) { return s.c_str(); }

template<class T, class StringType>
tabTableChtml* standardSearch2(const T& map, const char* urlPart, const char* title,
	StringType T::valueType::* namePtr)
{
	tabTableChtml* tp = new tabTableChtml;
	tabTableChtml& t(*tp);
	t.id = map.name;
	t.title = title;
	t.columns.push_back(Column(NAME, "Name", ENTRY, map.name));
	for(typename T::citr itr = map.begin();
		itr != map.end() && t.array.size() < MAX_COUNT;
		++itr)
	{
		const auto& s(itr->second);
		if(strcasestr(cstr(s.*namePtr), urlPart))
		{
			Row r;
			r[ENTRY] = toString(itr->first);
			r[NAME] = s.*namePtr;
			t.array.push_back(r);
		}
	}
	t.count = t.array.size();
	return tp;
}

template<class T>
tabTableChtml* standardSearch(const T& map, const char* urlPart, const char* title) {
	return standardSearch2(map, urlPart, title, &T::valueType::name);
}

void searchChtml::getResponse2(const char* u, DllResponseData* drd, ostream& os) {
	gWorldMapAreas.load();
	gAreaTable.load();
	gSpells.load();
	gItems.load();
	gNpcs.load();
	gAchievements.load();
	gObjects.load();
	gQuests.load();
	gTitles.load();
	gItemSets.load();
	gFactions.load();

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
	mTabs.push_back(standardSearch(gSpells, urlPart, "Spells"));
	mTabs.push_back(standardSearch(gItems, urlPart, "Items"));
	mTabs.push_back(standardSearch(gItemSets, urlPart, "Item Sets"));
	mTabs.push_back(standardSearch(gNpcs, urlPart, "NPCs"));
	mTabs.push_back(standardSearch(gObjects, urlPart, "Objects"));
	mTabs.push_back(standardSearch(gAchievements, urlPart, "Achievements"));
	mTabs.push_back(standardSearch(gTitles, urlPart, "Titles"));
	mTabs.push_back(standardSearch2(gQuests, urlPart, "Quests", &Quest::title));
	mTabs.push_back(standardSearch(gFactions, urlPart, "Factions"));
}

void searchChtml::title(ostream& stream) {
	ESCAPE(urlPart);
}
