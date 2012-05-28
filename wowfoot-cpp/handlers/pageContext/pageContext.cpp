#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "dllInterface.h"
#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include "pageContext.h"
#include "util/exception.h"

using namespace std;

void getResponse(const char* urlPart, DllResponseData* drd, PageContext& context) {
	ostringstream oss;
	try {
		drd->code = 0;
		context.getResponse2(urlPart, drd, oss);
		int code = context.run(oss);
		// set response code unless it was set by getResponse2().
		if(drd->code == 0)
			drd->code = code;
	} catch(Exception& e) {
		oss.str("");	//clear
		oss << "<pre>Internal Server Error:\n";
		e.stream(oss);
		drd->code = 500;
	} catch(exception& e) {
		printf("getResponse() caught std::exception: %s\n", e.what());
		oss.str("");	//clear
		oss << "<pre>Internal Server Error:<br>\n";
		oss << "Exception: "<<e.what();
		drd->code = 500;
		oss.flush();
		printf("oss.str().size(): %" PRIuPTR "\n", oss.str().size());
	}
	string* s = new string(oss.str());
	drd->size = s->size();
	drd->text = (void*)s->c_str();
	drd->user = s;
}

void PageContext::httpArgument(const char* key, const char* value) {
	throw Exception("httpArgument");
}

PageContext::PageContext(const char* pt) : pageTitle(pt) {}

int PageContext::run(ostream& os) {
	runHead(os);
	int res = runPage(os);
	runFoot(os);
	return res;
}

static int argc(void* user, const char* key, const char* value) {
	PageContext* pc = (PageContext*)user;
	pc->httpArgument(key, value);
	return 1;
}

// calls httpArgument.
void PageContext::getArguments(ResponseData* rd) {
	rd->getArgs(rd->getArgsSrc, argc, this);
}
