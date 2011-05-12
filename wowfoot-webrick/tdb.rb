#!/usr/bin/ruby
# open mysql connection to TDB
require 'rubygems'	# 1.8 bug workaround

require 'dbi'
require '../wowfoot-ex/output/WorldMapArea.rb'

#p DBI::available_drivers

dbc = DBI::connect('dbi:Mysql:world', 'trinity', 'trinity')

#query = 'SELECT entry, ZoneOrSort, RequiredRaces, MinLevel, title from quest_template LIMIT 0,100;'
#results = dbc.execute(query)

area = WORLD_MAP_AREA[14]	# Durotar, for starters
map = area[:map]
xMax = area[:a][:x]
yMax = area[:a][:y]
xMin = area[:b][:x]
yMin = area[:b][:y]

query = "SELECT creature.guid, creature.id, position_x, position_y, position_z, creature_questrelation.quest"+
	", creature_template.name"+
	" FROM creature"+
	" INNER JOIN creature_questrelation ON creature.id = creature_questrelation.id"+
	" INNER JOIN creature_template ON creature.id = creature_template.entry"+
	" WHERE map = #{map}"+
	" AND position_x BETWEEN #{xMin} AND #{xMax}"+
	" AND position_y BETWEEN #{yMin} AND #{yMax}"+
	""

puts query

xDiff = xMax - xMin
yDiff = yMax - yMin

stmt = dbc.prepare(query)
stmt.execute
count = 0
stmt.fetch do |row|
	#p row
	
	# values between 0 and 1.
	x = (row[:position_x] - xMin) / xDiff
	y = (row[:position_y] - yMin) / yDiff
	z = row[:position_z]
	qid = row[:quest]
	id = row[:id]
	name = row[:name]
	guid = row[:guid]
	
	# write some sort of overlay image.
	width = 1024
	height = 768
	puts "<whatev src=\"dot.png\" x=#{(x*width).to_i} y=#{(y*height).to_i}> gu:#{guid} id:#{id} q:#{qid} z:#{z} name: #{name}"
	count += 1
end
puts "Got #{count} rows."
