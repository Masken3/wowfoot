
puts "Hello search!"
q = @id[1]
p q

stm = TDB::C.prepare('select entry, ZoneOrSort, SkillOrClassMask, MinLevel, QuestLevel'+
	', RequiredRaces, PrevQuestId, NextQuestId, title'+
	' from quest_template where title like ?')
stm.execute('%'+q+'%')
@res = stm.fetch_all

puts "#{@res.size} rows"
#p res

#p res[0][:entry]

puts "end"
STDOUT.flush
