
stm = TDB::C.prepare('select class, subclass, name, quality, sellprice'+
	', disenchantID'+
	' from item_template where entry = ?')
stm.execute(@id)
@template = stm.fetch

stm = TDB::C.prepare('select entry, item, chanceOrQuestChance, mincountOrRef, maxcount'+
	' from disenchant_loot_template dlt where entry = ?')
stm.execute(@template[:disenchantID])
@disenchantTo = stm.fetch_all
#p @disenchantTo
@disenchantTo = [] if(!@disenchantTo)	# temp hack

@TAB_TABLES = [
{
	:id => 'disenchantTo',
	:array => @disenchantTo,
	:title => 'Disenchants into',
	:columns => [
		['Name', :item, true],	# item name
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
	:id => 'disenchantFrom',
	:array => @disenchantFrom,
	:title => 'Disenchanted from',
	:columns => [
		['Name', :name, true],	# item name
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
