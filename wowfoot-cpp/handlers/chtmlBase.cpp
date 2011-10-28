#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <dllInterface.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include "chtmlBase.h"

using namespace std;

void getResponse(const char* urlPart, DllResponseData* drd, PageContext& context) {
	ostringstream oss;
	try {
		context.getResponse2(urlPart, drd, oss);
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
	printf("cleanup. code %i. size: %"PRIu64"\n", drd->code, drd->size);
	string* s = (string*)drd->user;
	delete s;
}
