#ifndef FACTION_TEMPLATE_INDEX_H
#define FACTION_TEMPLATE_INDEX_H

#include "dbcFactionTemplate.h"
//#include "dbcFaction.h"
#include "dllHelpers.h"
#include <unordered_map>

using namespace std;

namespace FactionTemplateIndex {
	void load() VISIBLE;

	// finds faction IDs corresponding to a named faction ID.
	typedef unordered_multimap<int, FactionTemplates::citr> FactionMap;
	typedef FactionMap::const_iterator FactionItr;
	typedef pair<FactionItr, FactionItr> FactionPair;
	FactionPair findFaction(int entry) VISIBLE;
};

#endif	//FACTION_TEMPLATE_INDEX_H
