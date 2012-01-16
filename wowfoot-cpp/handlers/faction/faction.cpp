#define __STDC_FORMAT_MACROS
#include "faction.chtml.h"
#include "comments.h"

using namespace std;

void factionChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gFactions.load();
	//gNpcs.load();

	int id = toInt(urlPart);
	a = gFactions.find(id);
	if(a) {
		mTitle = a->name;

		mTabs.push_back(getComments("faction", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}

	drd->code = run(os);
}
