#define __STDC_FORMAT_MACROS
#include "FactionTemplate.index.h"
#include <inttypes.h>
#include <stdio.h>
#include <unordered_set>

using namespace std;
using namespace FactionTemplateIndex;

static FactionMap sFactionMap;
static bool sLoaded;

void FactionTemplateIndex::load() {
	if(sLoaded)
		return;
	sLoaded = true;

	gFactionTemplates.load();

	for(FactionTemplates::citr itr = gFactionTemplates.begin();
		itr != gFactionTemplates.end(); ++itr)
	{
		//printf("ft %i -> %i\n", itr->second.nameId, itr->first);
		sFactionMap.insert(pair<int, FactionTemplates::citr>(itr->second.nameId, itr));
	}

	printf("FactionTemplateIndex: Loaded %"PRIuPTR" rows into %s\n",
		sFactionMap.size(), "sFactionMap");
}

FactionPair FactionTemplateIndex::findFaction(int entry) {
	return sFactionMap.equal_range(entry);
}
