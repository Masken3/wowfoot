#include "itemset.chtml.h"
#include "comments.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

void itemsetChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gItemSets.load();
	gItems.load();

	int id = toInt(urlPart);
	a = gItemSets.find(id);
	if(a) {
		mTitle = a->name;
		mTabs.push_back(getComments("itemset", id));
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
	drd->code = run(os);
}
