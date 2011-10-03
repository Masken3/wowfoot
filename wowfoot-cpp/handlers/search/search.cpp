#include "search.chtml.h"
#include "dllInterface.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	WorldMapAreas_ensureLoad();
	AreaTable_ensureLoad();

	searchChtml context;
	context.urlPart = urlPart;

	string searchString = toupper(urlPart);

	for(AreaTable::citr itr = gAreaTable.begin(); itr != gAreaTable.end(); ++itr) {
		if(itr->second.parent == 0 &&
			toupper(itr->second.name).find(searchString) != string::npos)
		{
			context.areas.push_back(*itr);
		}
	}

	getResponse(drd, context);
}
