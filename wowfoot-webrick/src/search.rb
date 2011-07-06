
#puts "Hello search!"

searchTerm = @id[1]

@zones = {}
WORLD_MAP_AREA.each do |i, h|
	name = AREA_TABLE[i][:name]
	#puts "Test: '#{name}'.include?('#{searchTerm}')"
	@zones[i] = name if(name.upcase.include?(searchTerm.upcase))
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
	' from creature_template where name like ? or subname like ? LIMIT 0,100')
stm.execute(query, query)
@creatures = stm.fetch_all

stm = TDB::C.prepare('select entry, class, subclass, name, quality, sellprice'+
	' from item_template where name like ? LIMIT 0,100')
stm.execute(query)
@items = stm.fetch_all

stm = TDB::C.prepare('select entry, type, name, ScriptName'+
	' from gameobject_template where name like ? LIMIT 0,100')
stm.execute(query)
@gobjects = stm.fetch_all
