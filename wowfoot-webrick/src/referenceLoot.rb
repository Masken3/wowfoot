#stm = TDB::C.prepare('select * from reference_loot_template')
#stm.execute
#@RLT = {}
#stm.fetch_all do |row|
#	@RLT[row[:entry]] = row
#end

# lt = Loot Template
# st = Source Template
def getStandardLoot(id, lt, st = lt)
	stm = TDB::C.prepare('select st.entry, st.name, llt.chanceOrQuestChance, llt.mincountOrRef, llt.maxcount'+
	" from #{st}_template st"+
	" INNER JOIN #{lt}_loot_template llt on llt.entry = st.entry"+
	' where llt.item = ? AND llt.mincountOrRef >= 0')
	stm.execute(@id)
	part1 = stm.fetch_all
	
	stm = TDB::C.prepare('select st.entry, st.name, rlt.chanceOrQuestChance, rlt.mincountOrRef, rlt.maxcount'+
	" from #{st}_template st"+
	" INNER JOIN #{lt}_loot_template llt on llt.entry = st.entry"+
	' INNER JOIN reference_loot_template rlt on (-llt.mincountOrRef) = rlt.entry'+
	' where rlt.item = ?')
	stm.execute(@id)
	part2 = stm.fetch_all
	
	return part1 + part2
end
