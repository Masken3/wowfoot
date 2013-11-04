#ifndef SPELL_SHARED_H
#define SPELL_SHARED_H

#include "itrPair.h"
#include "dbcSpell.h"

class Tab;
class Quality;

int slaYellow(int id);
int streamSource(ostream& stream, int id);
const Quality* streamMultiItem(ostream& stream, int id, int count);
Tab* spellsTab(const char* tabId, const char* tabTitle, ItrPair<Spell>&,
	bool haveSkillName) VISIBLE;

#endif	//SPELL_SHARED_H
