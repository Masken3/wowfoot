#ifndef QUESTTABLE_H
#define QUESTTABLE_H

#include "tabTable.h"
#include "db_quest.h"

void questColumns(tabTableChtml& t) VISIBLE;
void questRow(Row& r, const Quest& q) VISIBLE;
Tab* getQuestsTab(const char* id, const char* title, int entry, Quests::IntPair (Quests::*finder)(int)const) VISIBLE;

#endif	//QUESTTABLE_H
