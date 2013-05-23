@structName = 'QuestRelation'

@tables = [
	Table.new('creature_questrelation', 'CreatureQuestGivers'),
	Table.new('creature_involvedrelation', 'CreatureQuestFinishers'),
	Table.new('gameobject_questrelation', 'ObjectQuestGivers'),
	Table.new('gameobject_involvedrelation', 'ObjectQuestFinishers'),
]

@containerType = :set
@struct = [
c(:int, :id, :key),
c(:int, :quest, :key),
]

@index = [
	[:id],
	[:quest],
]
