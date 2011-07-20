
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
#	'reference',	# what's this?
	'skinning',
#	'spell',	# incomplete
]
#todo: vendor, quest

loot_entries = ''
loot_joins = ''
loots.each do |l|
	loot_entries += ", #{l[0..2]}lt.entry as #{l}Loot"
	# works if you have an index on ?lt.item.
	loot_joins += " LEFT JOIN #{l}_loot_template #{l[0..2]}lt on #{l[0..2]}lt.item = it.entry"
end

stm = TDB::C.prepare('select it.entry, it.name, dlt.chanceOrQuestChance, dlt.mincountOrRef, dlt.maxcount'+
	loot_entries +
	' from item_template it'+
	' INNER JOIN disenchant_loot_template dlt on dlt.entry = it.disenchantID'+
	loot_joins +
	' where dlt.item = ?'+
	' GROUP BY it.entry')
stm.execute(@id)
@disenchantFrom = stm.fetch_all

@disenchantFrom.each do |row|
	sources = ''
	loots.each do |l|
		if(row[l+'Loot'])
			sources += ', ' if(!sources.empty?)
			sources += l
		end
	end
	# hack because row is not a hash.
	row[:itemLoot] = sources
end

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

stm = TDB::C.prepare('select gt.entry, gt.name, glt.chanceOrQuestChance, glt.mincountOrRef, glt.maxcount'+
	' from gameobject_template gt'+
	' INNER JOIN gameobject_loot_template glt on glt.entry = gt.entry'+
	' where glt.item = ?')
stm.execute(@id)
@object = stm.fetch_all

stm = TDB::C.prepare('select it.entry, it.name, ilt.chanceOrQuestChance, ilt.mincountOrRef, ilt.maxcount'+
	' from item_template it'+
	' INNER JOIN item_loot_template ilt on ilt.entry = it.entry'+
	' where ilt.item = ?')
stm.execute(@id)
@contained = stm.fetch_all

questColumns = {
:provided => {:name => 'SrcItemId', :title => 'Provided for quest'},
:required => {:name => 'ReqItem', :title => 'Required for quest'},	# for successful completion
:rewardChoice => {:name => 'RewChoiceItemId', :title => 'Reward choice'},
:reward => {:name => 'RewItemId', :title => 'Reward'},
}
questColumns.each do |id, hash|
	hash[:names] = []
end

# todo: move to util file
class String
	def startsWith?(str)
		return slice(0, str.length) == str
	end
end

# make sure all relevant columns are indexed, lest queries take too long.
stm = TDB::C.prepare('describe quest_template')
stm.execute()
stm.fetch_all.each do |row|
	f = row[:Field]
	if(f.include?('ItemId'))
		#p f
		questColumns.each do |id, hash|
			hash[:names] << f if(f.startsWith?(hash[:name]))
		end
		if(row[:Key] == "")
			puts "Creating index for column: #{f}"
			stm = TDB::C.prepare("CREATE INDEX #{f} ON quest_template (#{f})")
			stm.execute
		end
	end
end
questColumns.each do |id, hash|
	names = nil
	hash[:names].each do |n|
		if(!names)
			names = ''
		else
			names += ' OR'
		end
		names += " qt.#{n} = ?"
	end
	stm = TDB::C.prepare('select qt.entry, qt.title'+
		' from quest_template qt'+
		' where '+names)
	#p names
	params = Array.new(hash[:names].size, @id)
	#p params
	stm.execute(*params)
	hash[:entries] = stm.fetch_all
end
questTables = questColumns.collect do |id, hash|
{
	:id => id,
	:array => hash[:entries],
	:title => hash[:title],
	:columns => [
		['Entry', :entry],
		['Title', :title, :entry, 'quest'],
	],
}
end
#p questTables

# column format: [title, array key, link array key, link page name]
# link parts are optional.
@TAB_TABLES = questTables + [
{
	:id => 'object',
	:array => @object,
	:title => 'Contained in object',
	:columns => [
		['Name', :name, :entry, 'object'],
		['Location', :location],	# zone & area
		['Chance', :chanceOrQuestChance],
		['MinCount', :mincountOrRef],
		['MaxCount', :maxcount],
	],
},
{
	:id => 'contained',
	:array => @contained,
	:title => 'Contained in item',
	:columns => [
		['Name', :name, :entry, 'item'],
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
		['Sources', :itemLoot],
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
