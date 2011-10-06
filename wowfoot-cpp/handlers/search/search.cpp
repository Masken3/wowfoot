#include "search.chtml.h"
#include "dllInterface.h"
#include "tabTables.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

enum TableId {
	ZONE,
};
enum TableRowId {
	NAME = ENTRY+1,
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
		const Area& a(itr->second);
		if(a.parent == 0 &&
			toupper(a.name).find(searchString) != string::npos)
		{
			Row r;
			r[ENTRY] = toString(itr->first);
			r[NAME] = a.name;
			t.array.push_back(r);
		}
	}
	context.mTables.push_back(t);

	getResponse(drd, context);
}
