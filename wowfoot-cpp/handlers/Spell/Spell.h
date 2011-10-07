#ifndef SPELL_H
#define SPELL_H

#include "ConstMap.h"
#include "Spell.struct.h"
#include "dllHelpers.h"

class Spells : public ConstMap<int, Spell> {
public:
	void load() VISIBLE;
};

extern Spells gSpells VISIBLE;

#endif	//SPELL_H
