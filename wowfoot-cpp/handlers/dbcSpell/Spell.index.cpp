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
				printf("learn: %i teaches %i\n", s.id, e.triggerSpell);
				insert(sLearnSpellMap, e.triggerSpell, &s);
			}
		}
	}

	printf("SpellIndex: Loaded %" PRIuPTR " rows into %s\n",
		sLearnSpellMap.size(), "sLearnSpellMap");
}

SpellPair SpellIndex::findLearnSpell(int entry) {
	return sLearnSpellMap.equal_range(entry);
}
