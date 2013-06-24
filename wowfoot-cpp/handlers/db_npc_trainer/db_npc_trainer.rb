@mysql_db_name = 'npc_trainer'
@structName = 'NpcTrainer'
@containerType = :set

@struct = [
c(:int, :entry, :key),
c(:int, :spell, :key),
c(:int, :spellCost),
c(:int, :reqSkill),
c(:int, :reqSkillValue),
c(:int, :reqLevel),
]

@index = [
	[:entry],
	[:spell],
]
