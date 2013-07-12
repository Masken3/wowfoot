#ifndef LOCK_INDEX_H
#define LOCK_INDEX_H

#include "dbcLock.h"
#include "dllHelpers.h"
#include "db_item.struct.h"
#include "db_creature_template.struct.h"
#include "db_gameobject_template.struct.h"
#include <unordered_map>

using namespace std;

namespace LockIndex {
	void load() VISIBLE;

	// find locks of the specified type.
	typedef unordered_multimap<int, const Lock*> LockMap;
	typedef LockMap::const_iterator LockItr;
	typedef pair<LockItr, LockItr> LockPair;
	LockPair findLock(int type) VISIBLE;

	// find objects that the specified lock type can open.
	typedef unordered_multimap<int, const Object*> ObjectMap;
	typedef ObjectMap::const_iterator ObjectItr;
	typedef pair<ObjectItr, ObjectItr> ObjectPair;
	ObjectPair findObject(int type) VISIBLE;

	// finds NPCs that the specified lock type can open.
	typedef unordered_multimap<int, const Npc*> NpcMap;
	typedef NpcMap::const_iterator NpcItr;
	typedef pair<NpcItr, NpcItr> NpcPair;
	NpcPair findNpc(int type) VISIBLE;

	// finds items that the specified lock type can open.
	typedef unordered_multimap<int, const Item*> ItemMap;
	typedef ItemMap::const_iterator ItemItr;
	typedef pair<ItemItr, ItemItr> ItemPair;
	ItemPair findItem(int type) VISIBLE;
};

#endif	//LOCK_INDEX_H
