#define __STDC_FORMAT_MACROS
#include "SkillLineAbility.index.h"
#include <inttypes.h>
#include <stdio.h>
#include <unordered_set>
#include "util/criticalSection.h"
#include "util/stl_map_insert.h"

using namespace std;
using namespace SkillLineAbilityIndex;

static CriticalSectionLoadGuard sCS;
static SpellMap sSpellMap;
static SpellMap sSkillMap;

void SkillLineAbilityIndex::load() {
	LOCK_AND_LOAD;

	gSkillLineAbilities.load();

	for(auto itr = gSkillLineAbilities.begin(); itr != gSkillLineAbilities.end(); ++itr)
	{
		sSpellMap.insert(make_pair(itr->second.spell, &itr->second));
		sSkillMap.insert(make_pair(itr->second.skill, &itr->second));
		//printf("skill %i, spell %i\n", itr->second.skill, itr->second.spell);
	}

	printf("SkillLineAbilityIndex: Loaded %" PRIuPTR " rows into %s\n",
		sSpellMap.size(), "sSpellMap and sSkillMap");
}

SpellPair SkillLineAbilityIndex::findSpell(int id) {
	EASSERT(!sSpellMap.empty());
	return sSpellMap.equal_range(id);
}

SpellPair SkillLineAbilityIndex::findSkill(int id) {
	EASSERT(!sSkillMap.empty());
	return sSkillMap.equal_range(id);
}
