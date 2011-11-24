def spawnCount(spawnDb, templateStruct, perTemplateCode = '')
s = spawnDb.capitalize

@extraMembers = [
'int spawnCount;',
]

@extraHeaderCode = %q(
#include "db_spawn.h")

@extraInitCode = "
g#{s}Spawns.load();
int totalSpawnCount = 0;
for(#{templateStruct}s::iterator itr = super::begin(); itr != super::end(); ++itr) {
	#{templateStruct}& t(itr->second);
	Spawns::IdPair p = g#{s}Spawns.findId(t.entry);
	t.spawnCount = 0;
	for(; p.first != p.second; ++p.first) {
		t.spawnCount++;
		totalSpawnCount++;
	}#{perTemplateCode}
}
printf(\"Counted %i #{spawnDb} spawn points\\n\", totalSpawnCount);"
end
