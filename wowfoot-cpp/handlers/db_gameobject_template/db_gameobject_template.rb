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
]

@index = [
]

@extraClassDefinitionCode = %q(
	typedef unordered_multimap<int, const Object*> LootMap;
	typedef LootMap::const_iterator LootItr;
	typedef pair<LootItr, LootItr> LootPair;
private:
	LootMap mLootMap;
public:
	LootPair findLoot(int id) const VISIBLE {
		return mLootMap.equal_range(id);
	}

private:
	LootMap mLockMap;
public:
	LootPair findLock(int lockId) const VISIBLE {
		return mLockMap.equal_range(lockId);
	}
)

require './handlers/db_spawn/spawnCount.rb'
# 3 - chest
# 25 - fishing hole
# 3.1 - lootId
# 25.1 - lootId
# 3.0 - lockId
# 25.4 - lockId
spawnCount('gameobject', 'Object', %q(
	if(t.type == 3 || t.type == 25) {
		mLootMap.insert(pair<int, const Object*>(t.data1, &t));
		int lockId = (t.type == 3) ? t.data0 : t.data4;
		mLockMap.insert(pair<int, const Object*>(lockId, &t));
	}
))

@extraInitCode << %q(
printf("Loaded %" PRIuPTR " rows into %s\n", mLootMap.size(), "mLootMap");
printf("Loaded %" PRIuPTR " rows into %s\n", mLockMap.size(), "mLockMap");
)
