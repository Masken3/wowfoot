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

query = "SELECT id, position_x, position_y, position_z FROM creature WHERE map = #{map}"+
	" AND position_x >= #{xMin} AND position_x <= #{xMax}"+
	" AND position_y >= #{yMin} AND position_y <= #{yMax}"

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
	
	# write some sort of overlay image.
	width = 1024
	height = 768
	#puts "<whatev src=\"dot.png\" x=#{x*width} y=#{y*height}>"
	count += 1
end
puts "Got #{count} rows."
