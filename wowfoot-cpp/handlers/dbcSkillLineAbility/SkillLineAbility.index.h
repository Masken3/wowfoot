#ifndef SKILL_LINE_ABILITY_INDEX_H
#define SKILL_LINE_ABILITY_INDEX_H

#include "dbcSkillLineAbility.h"
#include "dllHelpers.h"
#include <unordered_map>

using namespace std;

namespace SkillLineAbilityIndex {
	void load() VISIBLE;

	typedef unordered_multimap<int, const SkillLineAbility*> SpellMap;
	typedef SpellMap::const_iterator SpellItr;
	typedef pair<SpellItr, SpellItr> SpellPair;
	SpellPair findSpell(int id) VISIBLE;
};

#endif	//SKILL_LINE_ABILITY_INDEX_H
