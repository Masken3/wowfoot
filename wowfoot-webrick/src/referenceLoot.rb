#stm = TDB::C.prepare('select * from reference_loot_template')
#stm.execute
#@RLT = {}
#stm.fetch_all do |row|
#	@RLT[row[:entry]] = row
#end

# lt = Loot Template
# st = Source Template
# ct = Count Table
def getStandardLoot(id, lt, st = lt, ct = nil)
	sql = 'select st.entry, st.name, llt.chanceOrQuestChance, llt.mincountOrRef, llt.maxcount'
	if(ct)
		sql += ", (select count(*) from #{ct} where #{ct}.id = st.entry) as count"
	end
	sql += " from #{st}_template st"+
	" INNER JOIN #{lt}_loot_template llt on llt.entry = st.entry"+
	' where llt.item = ? AND llt.mincountOrRef >= 0'+
	' order by -llt.chanceOrQuestChance'
	stm = TDB::C.prepare(sql)
	stm.execute(@id)
	part1 = stm.fetch_all

	sql = 'select st.entry, st.name, rlt.chanceOrQuestChance, rlt.mincountOrRef, rlt.maxcount'
	if(ct)
		sql += ", (select count(*) from #{ct} where #{ct}.id = st.entry) as count"
	end
	sql += " from #{st}_template st"+
	" INNER JOIN #{lt}_loot_template llt on llt.entry = st.entry"+
	' INNER JOIN reference_loot_template rlt on (-llt.mincountOrRef) = rlt.entry'+
	' where rlt.item = ?'
	stm = TDB::C.prepare(sql)
	stm.execute(@id)
	part2 = stm.fetch_all

	return part1 + part2
end
