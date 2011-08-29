# todo: move to ww.rb when stable
run 'referenceLoot.rb'
run 'itemClass.rb'
run 'itemEquip.rb'
run 'itemQuality.rb'

stm = TDB::C.prepare('select class, subclass, name, quality, sellprice'+
	', disenchantID'+
	', inventoryType'+
	' from item_template where entry = ?')
stm.execute(@id)
@template = stm.fetch

raise HTTPStatus[404], "Item not found in database" if(!@template)


@itemClass = ITEM_CLASS[@template[:class].to_i]
if(@itemClass)
	@itemSubClass = @itemClass[:subclass][@template[:subclass].to_i]
	@itemSubClass = 'Unknown' if(!@itemSubClass)
else
	@itemClass = {:name => 'Unknown'}
	@itemSubClass = 'Unknown'
end

@quality = ITEM_QUALITY[@template[:quality].to_i]


stm = TDB::C.prepare('select dlt.entry, item, name, chanceOrQuestChance, mincountOrRef, dlt.maxcount'+
	' from disenchant_loot_template dlt'+
	' INNER JOIN item_template it on dlt.item = it.entry'+
	' where dlt.entry = ?')
stm.execute(@template[:disenchantID])
@disenchantTo = stm.fetch_all
#p @disenchantTo
@disenchantTo = [] if(!@disenchantTo)	# temp hack

@milledFrom = getStandardLoot(@id, 'milling', 'item')

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

@drop = getStandardLoot(@id, 'creature', 'creature', 'creature')

@pickpocket = getStandardLoot(@id, 'pickpocketing', 'creature', 'creature')

stm = TDB::C.prepare('select gt.entry, gt.name, llt.chanceOrQuestChance, llt.mincountOrRef, llt.maxcount'+
	", (select count(*) from gameobject where gameobject.id = gt.entry) as count"+
	" from gameobject_template gt"+
	" INNER JOIN gameobject_loot_template llt on llt.entry = gt.data1"+
	' where llt.item = ? AND (gt.type = 3 OR gt.type = 25)')	# chest or fishinghole
stm.execute(@id)
@object = stm.fetch_all

@contained = getStandardLoot(@id, 'item')

stm = TDB::C.prepare('select it.entry, it.name, chanceOrQuestChance, mincountOrRef, ilt.maxcount'+
	' from item_loot_template ilt'+
	' INNER JOIN item_template it on ilt.item = it.entry'+
	' where ilt.entry = ?')
stm.execute(@id)
@contains = stm.fetch_all

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
	:id => 'milled',
	:array => @milledFrom,
	:title => 'Milled from',
	:columns => [
		['Name', :name, :entry, 'item'],
		['Chance', :chanceOrQuestChance],
		['MinCount', :mincountOrRef],
		['MaxCount', :maxcount],
	],
},
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
		['Spawn count', :count],
	],
},
{
	:id => 'contains',
	:array => @contains,
	:title => 'Contains',
	:columns => [
		['Name', :name, :entry, 'item'],
		['Chance', :chanceOrQuestChance],
		['MinCount', :mincountOrRef],
		['MaxCount', :maxcount],
	],
},{
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
		['Spawn count', :count],
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
		['Spawn count', :count],
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

#p Array.instance_methods
@TAB_TABLES.reject! do |tab|
	tab[:array].size == 0
end

# unused
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
	:id => 'seller',
	:array => @seller,
	:title => 'Sold by',
	:columns => [
		['Name', :name, true],	# creature name
		['Location', :location],	# zone & area
	],
},
]
