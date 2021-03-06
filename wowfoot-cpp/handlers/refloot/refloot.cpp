#include "refloot.chtml.h"
#include "item_shared.h"
#include "db_item.h"
#include "db_loot_template.h"
#include "wowVersion.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

static Tab* refloot(int lootId);

void reflootChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gReferenceLoots.load();
	gItems.load();
	gCreatureLoots.load();
	gPickpocketingLoots.load();
	gSkinningLoots.load();
	gDisenchantLoots.load();
	gItemLoots.load();
	gGameobjectLoots.load();

	int id = toInt(urlPart);
	auto a = gReferenceLoots.findEntry(id);
	if(a.first != a.second) {
		mFound = true;
		mTabs.push_back(refloot(id));
		referentialLoots(mTabs, id);
	} else {
		mFound = false;
		drd->code = 404;
	}
	mTitle = urlPart;
}

void reflootChtml::title(ostream& stream) {
	ESCAPE(mTitle);
}

static Tab* refloot(int lootId) {
	return simpleLoot(lootId, "loot", "Loot", gReferenceLoots);
}
