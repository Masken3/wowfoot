@mysql_db_name = '_loot_template'
@structName = 'Loot'
@containerType = :set

@prefixes = [
	'creature',
	'disenchant',
	'fishing',
	'gameobject',
	'item',
	'pickpocketing',
	'prospecting',
	'skinning',
	'quest_mail',
	'reference',
	'milling',
]

@struct = [
c(:int, :entry, :key),
c(:int, :item, :key),
renamed(:float, :chanceOrQuestChance, :chance),
c(:int, :groupId),
c(:int, :minCountOrRef),
c(:int, :maxCount),
]

@index = [
	[:entry],
	[:item],
	[:groupId],
]
