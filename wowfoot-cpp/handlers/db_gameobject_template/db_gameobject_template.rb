@mysql_db_name = 'gameobject_template'
@structName = 'Object'

@struct = [
c(:int, :entry),
c(:int, :type),
c(:string, :name),
c(:int, :data0),
c(:int, :data1),
c(:int, :data2),
c(:int, :data3),
c(:int, :data4),
c(:int, :data7),
c(:int, :data10),
]

@index = [
]

@extraClassDefinitionCode = %q(
	typedef unordered_multimap<int, const Object*> IntObjectMap;
	typedef IntObjectMap::const_iterator IntObjectItr;
	typedef pair<IntObjectItr, IntObjectItr> IntObjectPair;
private:
	IntObjectMap mLootMap;
public:
	IntObjectPair findLoot(int id) const VISIBLE {
		return mLootMap.equal_range(id);
	}

private:
	IntObjectMap mLockMap;
public:
	IntObjectPair findLock(int lockId) const VISIBLE {
		return mLockMap.equal_range(lockId);
	}

private:
	IntObjectMap mSpellMap;
public:
	IntObjectPair findSpell(int spellId) const VISIBLE {
		return mSpellMap.equal_range(spellId);
	}

private:
	IntObjectMap mFocusMap;
public:
	IntObjectPair findFocus(int focusId) const VISIBLE {
		return mFocusMap.equal_range(focusId);
	}
)

require './handlers/db_spawn/spawnCount.rb'
# 3 - chest
# 6 - trap
# 8 - focus
# 10 - goober
# 18 - summoning ritual
# 22 - spellcaster
# 25 - fishing hole

# 3.1 - lootId
# 25.1 - lootId
# 3.0 - lockId
# 25.4 - lockId

# 6.3 - spellId
# 10.10 - spellId
# 18.1 - spellId
# 18.2 - animSpell
# 18.4 - casterTargetSpell
# 22.0 - spellId

# 8.0 - focusId
spawnCount('gameobject', 'Object', %q(
	if(t.type == 3 || t.type == 25) {
		mLootMap.insert(pair<int, const Object*>(t.data1, &t));
		int lockId = (t.type == 3) ? t.data0 : t.data4;
		mLockMap.insert(pair<int, const Object*>(lockId, &t));
	}
	int spellId = 0;
	if(t.type == 6) spellId = t.data3;
	if(t.type == 10) spellId = t.data10;
	if(t.type == 22) spellId = t.data0;
	if(t.type == 18) {
		spellId = t.data1;
		if(t.data2) ::insert(mSpellMap, t.data2, &t);
		if(t.data4) ::insert(mSpellMap, t.data4, &t);
	}
	if(spellId) {
		::insert(mSpellMap, spellId, &t);
	}
	if(t.type == 8) {
		::insert(mFocusMap, t.data0, &t);
	}
))

@extraHeaderCode << %q(
#include "util/stl_map_insert.h"
)

@extraInitCode << %q(
printf("Loaded %" PRIuPTR " rows into %s\n", mLootMap.size(), "mLootMap");
printf("Loaded %" PRIuPTR " rows into %s\n", mLockMap.size(), "mLockMap");
printf("Loaded %" PRIuPTR " rows into %s\n", mSpellMap.size(), "mSpellMap");
printf("Loaded %" PRIuPTR " rows into %s\n", mFocusMap.size(), "mFocusMap");
)
