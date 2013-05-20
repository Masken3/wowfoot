#include "zone.chtml.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

void zoneChtml::getResponse2(const char* u, DllResponseData* drd, ostream& os) {
	gWorldMapAreas.load();
	gAreaTable.load();

	urlPart = u;
	mId = toInt(urlPart);
	mWMA = gWorldMapAreas.find(mId);
	mAT = gAreaTable.find(mId);
	if(mWMA) {
		if(mAT) {
			mTitle = mAT->name;
		} else {
			mTitle = mWMA->name;
		}
	} else {
		mTitle = urlPart;
	}
}

void zoneChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}
