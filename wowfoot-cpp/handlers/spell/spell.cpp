#define __STDC_FORMAT_MACROS
#include "spell.chtml.h"
#include "comments.h"
#include "db_creature_template.h"
#include "money.h"
#include "exSpell.h"

#if 0
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <inttypes.h>
#endif

using namespace std;

extern "C"
void getResponse(const char* urlPart, DllResponseData* drd) {
	gNpcs.load();
	gItems.load();
	gSpells.load();

	spellChtml context;

	int id = toInt(urlPart);
	const Spell* a = context.a = gSpells.find(id);
	if(a) {
		context.mTitle = context.a->name;
		context.mTabs.push_back(getComments("spell", id));
	} else {
		context.mTitle = urlPart;
		drd->code = 404;
	}

	getResponse(drd, context);
}
