#ifndef DBC_SPELL_H
#define DBC_SPELL_H

#include "ConstMap.h"
#include "dllHelpers.h"
#include <unordered_map>

using namespace std;

struct Spell {
	const char* name;
	const char* rank;
	const char* description;
	const char* toolTip;
};

class Spells : public ConstMap<int, Spell> {
public:
	void load() VISIBLE;
};

extern Spells gSpells VISIBLE;

#endif	//DBC_SPELL_H
