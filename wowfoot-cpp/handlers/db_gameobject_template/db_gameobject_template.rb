@mysql_db_name = 'gameobject_template'
@structName = 'Object'

@struct = [
c(:int, :entry),
c(:int, :type),
c(:string, :name),
c(:int, :data1),
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
)

require './handlers/db_spawn/spawnCount.rb'
spawnCount('gameobject', 'Object', %q(
	if(t.type == 3 || t.type == 25)
		mLootMap.insert(pair<int, const Object*>(t.data1, &t));
))

@extraInitCode << %q(
printf("Loaded %"PRIuPTR" rows into %s\n", mLootMap.size(), "mLootMap");)
