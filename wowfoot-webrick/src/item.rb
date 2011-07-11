
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

stm = TDB::C.prepare('select it.entry, it.name, dlt.chanceOrQuestChance, dlt.mincountOrRef, dlt.maxcount'+
	' from item_template it'+
	' INNER JOIN disenchant_loot_template dlt on dlt.entry = it.disenchantID'+
	' where dlt.item = ?')
stm.execute(@id)
@disenchantFrom = stm.fetch_all

# column format: [title, array key, link array key, link page name]
# link parts are optional.
@TAB_TABLES = [
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
{
	:id => 'disenchantFrom',
	:array => @disenchantFrom,
	:title => 'Disenchanted from',
	:columns => [
		['Name', :name, :entry, 'item'],	# item name
		['Chance', :chanceOrQuestChance],
		['MinCount', :mincountOrRef],
		['MaxCount', :maxcount],
	],
},
]

@TAB_TABLES2 = [
'Disenchants into',
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
	:id => 'drop',
	:array => @drop,
	:title => 'Dropped by',
	:columns => [
		['Name', :name, true],	# creature name
		['Location', :location],	# zone & area
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
{
	:id => 'pickpocket',
	:array => @pickpocket,
	:title => 'Pickpocketed from',
	:columns => [
		['Name', :name, true],	# creature name
		['Location', :location],	# zone & area
	],
},
]
