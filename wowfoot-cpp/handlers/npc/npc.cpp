#include "npc.chtml.h"
#include "db_creature_template.h"
#include "db_creature.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>

using namespace std;

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	gWorldMapAreas.load();
	gAreaTable.load();
	gNpcs.load();
	gNpcSpawns.load();

	npcChtml context;
	int id = toInt(urlPart);
	const Npc* a = context.a = gNpcs.find(id);
	if(a) {
		context.mTitle = a->name.c_str();
	} else {
		context.mTitle = urlPart;
		drd->code = 404;
	}
	getResponse(drd, context);
}
