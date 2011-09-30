#include "ConstMap.h"

#include <dllInterface.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include "zone.chtml.h"
#include <sstream>

using namespace std;

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	WorldMapAreas_ensureLoad();
	AreaTable_ensureLoad();

	ostringstream oss;
	zoneChtml context;
	context.urlPart = urlPart;
	try {
		drd->code = context.run(oss);
	} catch(exception& e) {
		oss.str("");	//clear
		oss << "Internal Server Error:<br>\n";
		oss << "Exception: "<<e.what();
		drd->code = 500;
	}
	string* s = new string(oss.str());
	drd->size = s->size();
	drd->text = (void*)s->c_str();
	drd->user = s;
}

// must not attempt to delete[] or free() the drd itself.
extern "C"
void cleanup(DllResponseData* drd) {
#if 0
	free(drd->text);
#endif
	string* s = (string*)drd->user;
	delete s;
}
