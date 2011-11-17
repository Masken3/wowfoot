@mysql_db_name = 'creature_template'
@structName = 'Npc'

@struct = [
c(:int, :entry),
c(:string, :name),
c(:string, :subName),
c(:string, :iconName),
c(:int, :minLevel),
c(:int, :maxLevel),
c(:int, :exp),	# expansion
c(:int, :npcflag),
c(:int, :rank),
c(:float, :minDmg),
c(:float, :maxDmg),
c(:int, :dmgSchool),
c(:int, :attackPower),
c(:float, :dmg_multiplier),
c(:int, :baseAttackTime),
c(:int, :rangeAttackTime),
c(:int, :unit_flags),
c(:int, :dynamicFlags),
c(:int, :family),	# if type is Beast
c(:int, :trainer_type),
c(:int, :trainer_spell),
c(:int, :trainer_class),
c(:int, :trainer_race),
c(:float, :minRangeDmg),
c(:float, :maxRangeDmg),
c(:int, :rangedAttackPower),
c(:int, :type),
c(:int, :type_flags),
c(:int, :lootId),
c(:int, :pickpocketLoot),
c(:int, :skinLoot),
mc(:int, [:resistance], 6),
mc(:int, [:spell], 8),
c(:int, :petSpellDataId),
c(:int, :vehicleId),
c(:int, :minGold),
c(:int, :maxGold),
c(:string, :aiName),
c(:int, :movementType),
c(:int, :inhabitType),
c(:int, :racialLeader),
c(:int, :regenHealth),
c(:int, :equipment_id),
c(:int, :mechanic_immune_mask),
c(:int, :flags_extra),
c(:string, :scriptName),
]

@index = [
	[:lootId],
	[:pickpocketLoot],
	[:skinLoot],
]

@extraMembers = [
'int spawnCount;',
]

@extraHeaderCode = %q(
#include "db_creature.h")

@extraInitCode = %q(
gNpcSpawns.load();
int totalSpawnCount = 0;
for(Npcs::iterator itr = super::begin(); itr != super::end(); ++itr) {
	Npc& npc(itr->second);
	NpcSpawns::IdPair p = gNpcSpawns.findId(npc.entry);
	npc.spawnCount = 0;
	for(; p.first != p.second; ++p.first) {
		npc.spawnCount++;
		totalSpawnCount++;
	}
}
printf("Counted %i creature spawn points\n", totalSpawnCount);
)
