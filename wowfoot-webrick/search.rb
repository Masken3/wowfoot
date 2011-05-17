
#puts "Hello search!"

query = '%'+@id[1]+'%'

stm = TDB::C.prepare('select entry, ZoneOrSort, SkillOrClassMask, MinLevel, QuestLevel'+
	', RequiredRaces, PrevQuestId, NextQuestId, title'+
	' from quest_template where title like ? LIMIT 0,100')
stm.execute(query)
@quests = stm.fetch_all

#puts "#{@q.size} rows"
#p res
#p res[0][:entry]

stm = TDB::C.prepare('select entry, name, subname, minlevel, maxlevel, rank'+
	' from creature_template where name like ? LIMIT 0,100')
stm.execute(query)
@creatures = stm.fetch_all

#puts "end"
STDOUT.flush
