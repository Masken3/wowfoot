#ifndef SPELL_SHARED_H
#define SPELL_SHARED_H

#include "itrPair.h"
#include "dbcSpell.h"

class Tab;
class Quality;

int slaYellow(int id) VISIBLE;
int streamSource(ostream& stream, int id) VISIBLE;
const Quality* streamMultiItem(ostream& stream, int id, int count) VISIBLE;
Tab* spellsTab(const char* tabId, const char* tabTitle, ItrPair<Spell>&,
	bool haveSkillName) VISIBLE;

#endif	//SPELL_SHARED_H
