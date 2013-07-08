#ifndef PAGE_TAGS_H
#define PAGE_TAGS_H

#include "wowVersion.h"
#include "dbcSpell.h"
#include "dbcFaction.h"
#include "dbcWorldMapArea.h"
#include "db_item.h"
#include "db_gameobject_template.h"
#include "db_quest.h"
#include "db_creature_template.h"

#if CONFIG_WOW_VERSION > 30000
#include "dbcAchievement.h"
#define PAGE_TAG_ACHIEVEMENT(m) m("achievement", gAchievements)
#else
#define PAGE_TAG_ACHIEVEMENT(m)
#endif

#define PAGE_TAGS(m)\
	m("spell", gSpells)\
	m("item", gItems)\
	m("faction", gFactions)\
	m("npc", gNpcs)\
	PAGE_TAG_ACHIEVEMENT(m)\
	m("zone", gWorldMapAreas)\
	m("object", gObjects)\
	m("quest", gQuests)\

#endif	//PAGE_TAGS_H
