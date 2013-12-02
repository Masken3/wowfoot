#include "wowVersion.h"
#include "search.chtml.h"
#include "dllInterface.h"
#include "tabTable.h"
#include "dbcSpell.h"
#include "db_item.h"
#include "db_creature_template.h"
#include "db_gameobject_template.h"
#include "db_quest.h"
#include "dbcItemSet.h"
#include "dbcFaction.h"
#include "../itemShared/item_shared.h"

#if CONFIG_WOW_VERSION > 30000
#include "dbcAchievement.h"
#include "dbcCharTitles.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "win32.h"

using namespace std;

#define MAX_COUNT 1000

static const char* cstr(const char* s) { return s; }
static const char* cstr(const string& s) { return s.c_str(); }

template<class T> class ExtraColumn {
public:
	typedef void (*RowFunction)(Row&, const typename T::valueType&);
	RowFunction f;
	const char* colName;
	int colId;
};

static void spawnCountRowFunction(Row& r, const Npc& s) {
	r[SPAWN_COUNT] = toString(s.spawnCount);
	if(s.subName.length() > 0)
		r[NAME] += " <" + s.subName + ">";
}

static void spawnCountRowFunction(Row& r, const Object& s) {
	r[SPAWN_COUNT] = toString(s.spawnCount);
}

template<class T> class SpawnCountRow : public ExtraColumn<T> {
private:
public:
	SpawnCountRow() {
		ExtraColumn<T>::f = &spawnCountRowFunction;
		ExtraColumn<T>::colName = "Spawn count";
		ExtraColumn<T>::colId = SPAWN_COUNT;
	}
};

template<class T, class StringType>
void standardSearchPart(tabTableChtml& t, const T& map, const char* urlPart,
	StringType T::valueType::* namePtr, const ExtraColumn<T>* extra)
{
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
			if(extra) {
				extra->f(r, s);
			}
			t.array.push_back(r);
		}
	}
}

template<class T, class StringType>
tabTableChtml* standardSearch2(const T& map, const char* urlPart, const char* title,
	StringType T::valueType::* namePtr, const ExtraColumn<T>* extra = NULL)
{
	tabTableChtml* tp = new tabTableChtml;
	tabTableChtml& t(*tp);
	t.id = map.name;
	t.title = title;
	t.columns.push_back(Column(NAME, "Name", ENTRY, map.name));
	if(extra) {
		t.columns.push_back(Column(extra->colId, extra->colName));
	}
	standardSearchPart(t, map, urlPart, namePtr, extra);
	t.count = t.array.size();
	return tp;
}

template<class T>
tabTableChtml* standardSearch(const T& map, const char* urlPart, const char* title,
	const ExtraColumn<T>* extra = NULL)
{
	return standardSearch2(map, urlPart, title, &T::valueType::name, extra);
}

void searchChtml::getResponse2(const char* u, DllResponseData* drd, ostream& os) {
	gWorldMapAreas.load();
	gAreaTable.load();
	gSpells.load();
	gItems.load();
	gNpcs.load();
#if CONFIG_WOW_VERSION > 30000
	gAchievements.load();
	gTitles.load();
#endif
	gObjects.load();
	gQuests.load();
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
	standardSearchPart<WorldMapAreas, const char*>(t, gWorldMapAreas, urlPart, &WorldMapArea::name, NULL);
	t.count = t.array.size();
	mTabs.push_back(tp);
	}
	mTabs.push_back(standardSearch(gSpells, urlPart, "Spells"));
	mTabs.push_back(standardSearch(gItems, urlPart, "Items"));
	mTabs.push_back(standardSearch(gItemSets, urlPart, "Item Sets"));
	{
		SpawnCountRow<Npcs> scr;
		auto& t = *standardSearch(gNpcs, urlPart, "NPCs", &scr);
		standardSearchPart(t, gNpcs, urlPart, &Npc::subName, &scr);
		t.count = t.array.size();
		mTabs.push_back(&t);
	}
	{
		SpawnCountRow<Objects> scr;
		mTabs.push_back(standardSearch(gObjects, urlPart, "Objects", &scr));
	}
#if CONFIG_WOW_VERSION > 30000
	mTabs.push_back(standardSearch(gAchievements, urlPart, "Achievements"));
	mTabs.push_back(standardSearch(gTitles, urlPart, "Titles"));
#endif
	mTabs.push_back(standardSearch2(gQuests, urlPart, "Quests", &Quest::title));
	mTabs.push_back(standardSearch(gFactions, urlPart, "Factions"));
}

void searchChtml::title(ostream& stream) {
	ESCAPE(urlPart);
}
