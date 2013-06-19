#ifndef SPELL_INDEX_H
#define SPELL_INDEX_H

#include "dbcSpell.h"
#include "dllHelpers.h"
#include <unordered_map>

using namespace std;

namespace SpellIndex {
	void load() VISIBLE;

	// finds spells that teach the specified spell.
	typedef unordered_multimap<int, const Spell*> SpellMap;
	typedef SpellMap::const_iterator SpellItr;
	typedef pair<SpellItr, SpellItr> SpellPair;
	SpellPair findLearnSpell(int entry) VISIBLE;
};

#endif	//SPELL_INDEX_H
