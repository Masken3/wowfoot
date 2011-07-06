#!/usr/bin/ruby
# open mysql connection to TDB
require 'rubygems'	# 1.8 bug workaround

require 'dbi'
require '../wowfoot-ex/output/WorldMapArea.rb'

p DBI::available_drivers

dbc = DBI::connect('dbi:SQLite3:../wowfoot-import/imports.db')
stmt = dbc.prepare('SELECT COUNT(*) FROM comments')
stmt.execute
stmt.fetch do |row|
	p row
end
exit(0)


dbc = DBI::connect('dbi:Mysql:world', 'trinity', 'trinity')

#query = 'SELECT entry, ZoneOrSort, RequiredRaces, MinLevel, title from quest_template LIMIT 0,100;'
#results = dbc.execute(query)

area = WORLD_MAP_AREA[3483]	# Hellfire Peninsula, for starters
map = area[:map]
xMax = area[:a][:x]
yMax = area[:a][:y]
xMin = area[:b][:x]
yMin = area[:b][:y]

xDiff = xMax - xMin
yDiff = yMax - yMin

questStarters = false

if questStarters
query = "SELECT creature.guid, creature.id, position_x, position_y, position_z, creature_questrelation.quest"+
	", creature_template.name"+
	" FROM creature"+
	" INNER JOIN creature_questrelation ON creature.id = creature_questrelation.id"+
	" INNER JOIN creature_template ON creature.id = creature_template.entry"+
	" WHERE map = #{map}"+
	" AND position_x BETWEEN #{xMin} AND #{xMax}"+
	" AND position_y BETWEEN #{yMin} AND #{yMax}"+
	""
else
	CREATURE_ELITE_NORMAL = 0
	CREATURE_ELITE_ELITE = 1
	CREATURE_ELITE_RAREELITE = 2
	CREATURE_ELITE_WORLDBOSS = 3
	CREATURE_ELITE_RARE = 4

query = "SELECT creature.guid, creature.id, position_x, position_y, position_z"+
	", creature_template.name"+
	", creature_template.rank"+
	" FROM creature"+
	" INNER JOIN creature_template ON creature.id = creature_template.entry"+
	" WHERE map = #{map}"+
	" AND position_x BETWEEN #{xMin} AND #{xMax}"+
	" AND position_y BETWEEN #{yMin} AND #{yMax}"+
	" AND creature_template.rank IN (#{CREATURE_ELITE_RARE}, #{CREATURE_ELITE_RAREELITE})"+
	""
end

#itemQuery = "SELECT class, subclass, name, displayid, quality, sellPrice, 

puts query

stmt = dbc.prepare(query)
stmt.execute
count = 0
stmt.fetch do |row|
	#p row
	
	# values between 0 and 1.
	x = (row[:position_x] - xMin) / xDiff
	y = (row[:position_y] - yMin) / yDiff
	z = row[:position_z]
	if(questStarters)
	qid = row[:quest]
	end
	id = row[:id]
	name = row[:name]
	guid = row[:guid]
	
	# write some sort of overlay image.
	width = 1024
	height = 768
	if(questStarters)
	puts "<whatev src=\"dot.png\" x=#{(x*width).to_i} y=#{(y*height).to_i}> gu:#{guid} id:#{id} q:#{qid} z:#{z} name: #{name}"
	else
		rank = row[:rank]
	puts "<whatev src=\"dot.png\" x=#{(x*width).to_i} y=#{(y*height).to_i}> gu:#{guid} id:#{id} r:#{rank} z:#{z} name: #{name}"
	end
	count += 1
end
puts "Got #{count} rows."
