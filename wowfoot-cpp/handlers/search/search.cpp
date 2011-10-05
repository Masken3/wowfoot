#include "search.chtml.h"
#include "dllInterface.h"
#include "tabTables.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

enum {
	ZONE,
	ENTRY,
	NAME,
};

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	WorldMapAreas_ensureLoad();
	AreaTable_ensureLoad();

	searchChtml context;
	context.urlPart = urlPart;

	string searchString = toupper(urlPart);

	Table t;
	t.id = ZONE;
	t.title = "Zones";
	Column c = { NAME, "Name", false, true, ENTRY, "zone" };
	t.columns.push_back(c);

	for(AreaTable::citr itr = gAreaTable.begin(); itr != gAreaTable.end(); ++itr) {
		if(itr->second.parent == 0 &&
			toupper(itr->second.name).find(searchString) != string::npos)
		{
			context.areas.push_back(*itr);
			Row r;
			r.entry = itr->first;
			r[NAME] = itr->second.name;
			char buf[32];
			sprintf(buf, "%i", r.entry);
			r[ENTRY] = buf;
			t.array.push_back(r);
		}
	}
	context.mTables.push_back(t);

	getResponse(drd, context);
}
