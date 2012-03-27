#ifndef PAGE_TAGS_H
#define PAGE_TAGS_H

#include "dbcSpell.h"
#include "dbcAchievement.h"
#include "dbcFaction.h"
#include "dbcWorldMapArea.h"
#include "db_item.h"
#include "db_gameobject_template.h"
#include "db_quest.h"
#include "db_creature_template.h"

#define PAGE_TAGS(m)\
	m("spell", gSpells)\
	m("item", gItems)\
	m("faction", gFactions)\
	m("npc", gNpcs)\
	m("achievement", gAchievements)\
	m("zone", gWorldMapAreas)\
	m("object", gObjects)\
	m("quest", gQuests)\

#endif	//PAGE_TAGS_H
