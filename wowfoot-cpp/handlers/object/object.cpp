#include "object.chtml.h"
#include "db_gameobject_template.h"
#include "db_spawn.h"
#include "comments.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

void objectChtml::getResponse2(const char* urlPart, DllResponseData* drd, ostream& os) {
	gObjects.load();
	gGameobjectSpawns.load();

	int id = toInt(urlPart);
	a = gObjects.find(id);
	if(a) {
		mTitle = a->name.c_str();
		mTabs.push_back(getComments("object", id));

		mSpawnPointsChtml.mSpawns = gGameobjectSpawns.findId(id);
	} else {
		mTitle = urlPart;
		drd->code = 404;
	}
	drd->code = run(os);
}
