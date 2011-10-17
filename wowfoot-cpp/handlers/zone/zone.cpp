#include "zone.chtml.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	gWorldMapAreas.load();
	gAreaTable.load();

	zoneChtml context;
	context.urlPart = urlPart;
	getResponse(drd, context);
}
