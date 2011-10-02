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
	getResponse(drd, context);
}
