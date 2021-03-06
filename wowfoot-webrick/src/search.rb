run 'money.rb'

#puts "Hello search!"

run 'money.rb'

searchTerm = @id[1]

@zones = []
WORLD_MAP_AREA.each do |i, h|
	name = AREA_TABLE[i][:name]
	#puts "Test: '#{name}'.include?('#{searchTerm}')"
	@zones << {:entry => i, :name => name} if(name.upcase.include?(searchTerm.upcase))
end

# todo: prepare statements in advance.
query = '%'+searchTerm+'%'

stm = TDB::C.prepare('select entry, ZoneOrSort, SkillOrClassMask, MinLevel, QuestLevel'+
	', RequiredRaces, PrevQuestId, NextQuestId, title'+
	' from quest_template where title like ? LIMIT 0,100')
stm.execute(query)
@quests = stm.fetch_all

#puts "#{@q.size} rows"
#p res
#p res[0][:entry]

stm = TDB::C.prepare('select entry, name, subname, minlevel, maxlevel, rank'+
	', (select count(*) from creature where creature.id = ct.entry) as count'+
	' from creature_template ct where name like ? or subname like ? LIMIT 0,100')
stm.execute(query, query)
@creatures = stm.fetch_all
# todo: add spawn count

stm = TDB::C.prepare('select entry, class, subclass, name, quality, sellprice'+
	' from item_template where name like ? LIMIT 0,100')
stm.execute(query)
@items = stm.fetch_all

stm = TDB::C.prepare('select entry, type, name, ScriptName'+
	', (select count(*) from gameobject where gameobject.id = gt.entry) as count'+
	' from gameobject_template gt where name like ? LIMIT 0,100')
stm.execute(query)
@gobjects = stm.fetch_all

@items.each do |row|
	row[:sellprice] = moneyHtml(row[:sellprice])
end

# todo: use this to generate html.
@TAB_TABLES = [
{
	:id => 'zone',
	:array => @zones,
	:title => 'Zones',
	:columns => [
		['Name', :name, :entry, 'area'],
	],
},
{
	:id => 'quest',
	:array => @quests,
	:title => 'Quests',
	:columns => [
		['Zone', :questArea],
		['MinLevel', :MinLevel],
		['QuestLevel', :QuestLevel],
		['Title', :title, :entry, 'quest'],
	],
},
{
	:id => 'npc',
	:array => @creatures,
	:title => 'NPCs',
	:columns => [
		['MinLevel', :minlevel],
		['MaxLevel', :maxlevel],
		['Rank', :rank],
		['Name', :name, :entry, 'npc'],
		['Title', :subname],
		['Spawn count', :count],
	],
},
{
	:id => 'item',
	:array => @items,
	:title => 'Items',
	:columns => [
		['Class', :class],
		['Subclass', :subclass],
		['Quality', :quality],
		['Name', :name, :entry, 'item'],
		['Sell price', :sellprice, :noEscape],
	],
},
{
	:id => 'gobject',
	:array => @gobjects,
	:title => 'Objects',
	:columns => [
		['Type', :type],
		['Name', :name, :entry, 'object'],
		['Script', :ScriptName],
		['Spawn count', :count],
	],
},
]
