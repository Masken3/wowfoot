#define __STDC_FORMAT_MACROS
#include "Spell.index.h"
#include <inttypes.h>
#include <stdio.h>
#include <unordered_set>
#include "util/criticalSection.h"
#include "util/arraySize.h"
#include "util/stl_map_insert.h"
#include "win32.h"

using namespace std;
using namespace SpellIndex;

static CriticalSectionLoadGuard sCS;
static SpellMap sLearnSpellMap;
static SpellMap sSpawnCreatureSpellMap;
static SpellMap sReagentSpellMap;

void SpellIndex::load() {
	LOCK_AND_LOAD;

	gSpells.load();

	for(Spells::citr itr = gSpells.begin();
		itr != gSpells.end(); ++itr)
	{
		const Spell& s(itr->second);
		for(uint i=0; i<ARRAY_SIZE(s.effect); i++) {
			const Spell::Effect& e(s.effect[i]);
			if(e.id == 36) {	// Learn Spell
				//printf("learn: %i teaches %i\n", s.id, e.triggerSpell);
				insert(sLearnSpellMap, e.triggerSpell, &s);
			}
			if(e.id == 28 || // Summon
				e.id == 41 ||	// Summon Wild
				e.id == 42 ||	// Summon Guardian
				e.id == 56 ||	// Summon Pet
				e.id == 73 ||	// Summon Possessed
				e.id == 93 ||	// Summon Phantasm
				e.id == 97 ||	// Summon Critter
				e.id == 112 ||	// Summon Demon
				false)
			{
				insert(sSpawnCreatureSpellMap, e.miscValue, &s);
			}
		}
		for(uint i=0; i<ARRAY_SIZE(s.reagent); i++) {
			const Spell::Reagent& r(s.reagent[i]);
			insert(sReagentSpellMap, r.id, &s);
		}
	}

	printf("SpellIndex: Loaded %" PRIuPTR " rows into %s\n",
		sLearnSpellMap.size(), "sLearnSpellMap");
	printf("SpellIndex: Loaded %" PRIuPTR " rows into %s\n",
		sSpawnCreatureSpellMap.size(), "sSpawnCreatureSpellMap");
}

SpellPair SpellIndex::findLearnSpell(int entry) {
	EASSERT(!sLearnSpellMap.empty());
	return sLearnSpellMap.equal_range(entry);
}

SpellPair SpellIndex::findSpawnCreature(int entry) {
	EASSERT(!sSpawnCreatureSpellMap.empty());
	return sSpawnCreatureSpellMap.equal_range(entry);
}

SpellPair SpellIndex::findReagent(int entry) {
	EASSERT(!sReagentSpellMap.empty());
	return sReagentSpellMap.equal_range(entry);
}
