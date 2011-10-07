#include "zone.chtml.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	WorldMapAreas_ensureLoad();
	AreaTable_ensureLoad();

	zoneChtml context;
	context.urlPart = urlPart;
	getResponse(drd, context);
}