#include "Spell.data.h"
#include "Spell.h"

static bool sLoaded = false;

void Spells::load() {
	if(sLoaded)
		return;
	for(size_t i=0; i<gnSpell; i++) {
		insert(pair<int, Spell>(gSpell[i].id, gSpell[i].a));
	}
	sLoaded = true;
}

Spells gSpells;
