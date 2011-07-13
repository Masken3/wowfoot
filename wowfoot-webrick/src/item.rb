
stm = TDB::C.prepare('select class, subclass, name, quality, sellprice'+
	', disenchantID'+
	' from item_template where entry = ?')
stm.execute(@id)
@template = stm.fetch

stm = TDB::C.prepare('select dlt.entry, item, name, chanceOrQuestChance, mincountOrRef, dlt.maxcount'+
	' from disenchant_loot_template dlt'+
	' INNER JOIN item_template it on dlt.item = it.entry'+
	' where dlt.entry = ?')
stm.execute(@template[:disenchantID])
@disenchantTo = stm.fetch_all
#p @disenchantTo
@disenchantTo = [] if(!@disenchantTo)	# temp hack

loots = [
	'creature',
	'disenchant',
	'gameobject',
	'item',
	'milling',
	'pickpocketing',
	'prospecting',
	'reference',
	'skinning',
	'spell',
]

loot_entries = ''
loot_joins = ''
loots.each do |l|
	loot_entries += ", #{l[0..2]}lt.entry as #{l}Loot"
	loot_joins += " LEFT JOIN #{l}_loot_template #{l[0..2]}lt on #{l[0..2]}lt.item = it.entry"
end

stm = TDB::C.prepare('select it.entry, it.name, dlt.chanceOrQuestChance, dlt.mincountOrRef, dlt.maxcount'+
	', clt.entry as lootID'+
	', glt.entry as gameobjectLoot'+
	', plt.entry as pickpocketLoot'+
	' from item_template it'+
	' INNER JOIN disenchant_loot_template dlt on dlt.entry = it.disenchantID'+
	' LEFT JOIN creature_loot_template clt on clt.item = it.entry'+		# works if you have an index on clt.item.
	' LEFT JOIN gameobject_loot_template glt on glt.item = it.entry'+
	' LEFT JOIN pickpocketing_loot_template plt on plt.item = it.entry'+
	' where dlt.item = ?'+
	' GROUP BY it.entry')
stm.execute(@id)
@disenchantFrom = stm.fetch_all

stm = TDB::C.prepare('select ct.entry, ct.name, clt.chanceOrQuestChance, clt.mincountOrRef, clt.maxcount'+
	' from creature_template ct'+
	' INNER JOIN creature_loot_template clt on clt.entry = ct.lootID'+
	' where clt.item = ?')
stm.execute(@id)
@drop = stm.fetch_all

stm = TDB::C.prepare('select ct.entry, ct.name, plt.chanceOrQuestChance, plt.mincountOrRef, plt.maxcount'+
	' from creature_template ct'+
	' INNER JOIN pickpocketing_loot_template plt on plt.entry = ct.pickpocketLoot'+
	' where plt.item = ?')
stm.execute(@id)
@pickpocket = stm.fetch_all

# column format: [title, array key, link array key, link page name]
# link parts are optional.
@TAB_TABLES = [
{
	:id => 'disenchantFrom',
	:array => @disenchantFrom,
	:title => 'Disenchanted from',
	:columns => [
		['Name', :name, :entry, 'item'],	# item name
		['Chance', :chanceOrQuestChance],
		['MinCount', :mincountOrRef],
		['MaxCount', :maxcount],
		['Loot', :lootID],
		['Pickpocket', :pickpocketLoot],
		['Contained in object', :gameobjectLoot],
	],
},
{
	:id => 'pickpocket',
	:array => @pickpocket,
	:title => 'Pickpocketed from',
	:columns => [
		['Name', :name, :entry, 'npc'],	# creature name
#		['Location', :location],	# zone & area
		['Chance', :chanceOrQuestChance],
		['MinCount', :mincountOrRef],
		['MaxCount', :maxcount],
	],
},
{
	:id => 'drop',
	:array => @drop,
	:title => 'Dropped by',
	:columns => [
		['Name', :name, :entry, 'npc'],	# creature name
		['Chance', :chanceOrQuestChance],
		['MinCount', :mincountOrRef],
		['MaxCount', :maxcount],
	],
},
{
	:id => 'disenchantTo',
	:array => @disenchantTo,
	:title => 'Disenchants into',
	:columns => [
		['Name', :name, :item, 'item'],	# item name
		['Chance', :chanceOrQuestChance],
		['MinCount', :mincountOrRef],
		['MaxCount', :maxcount],
	],
},
]

@TAB_TABLES2 = [
'Contains',
{
	:id => 'reagent',
	:array => @reagent,
	:title => 'Reagent for',
	:columns => [
		['Name', :name],	# name of the spell
		['MinLevel', :MinLevel],	# clvl or plvl
		['Cost', :const],	# including any other reagents
		['Result', :result],	# spell effect (damage, buff application, item creation)
	],
},
{
	:id => 'create',
	:array => @create,
	:title => 'Created by',
	:columns => [
		['Name', :name],	# name of the spell
		['MinLevel', :MinLevel],	# clvl or plvl
		['Cost', :const],	# including any other reagents
		['Result', :result],	# spell effect (damage, buff application, item creation)
	],
},
{
	:id => 'object',
	:array => @object,
	:title => 'Contained in object',
	:columns => [
		['Name', :name, true],
		['Location', :location],	# zone & area
	],
},
{
	:id => 'contained',
	:array => @contained,
	:title => 'Contained in item',
	:columns => [
		['Name', :name, true],
	],
},
{
	:id => 'prospect',
	:array => @prospect,
	:title => 'Prospected from',
	:columns => [
		['Name', :name, true],
	],
},
{
	:id => 'mill',
	:array => @mill,
	:title => 'Milled from',
	:columns => [
		['Name', :name, true],
	],
},
{
	:id => 'seller',
	:array => @seller,
	:title => 'Sold by',
	:columns => [
		['Name', :name, true],	# creature name
		['Location', :location],	# zone & area
	],
},
]
