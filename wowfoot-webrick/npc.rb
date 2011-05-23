
# todo: prepare statements in advance.
stm = TDB::C.prepare('select entry, name, subname, minlevel, maxlevel, rank'+
	' from creature_template where entry = ?')
stm.execute(@id)
@template = stm.fetch

stm = TDB::C.prepare('select guid, id, map, position_x, position_y, position_z'+
	' from creature where id = ?')
stm.execute(@id)
@coords = stm.fetch_all

def zoneFromCoords(map, x, y)
	result = nil
	count = 0
	WORLD_MAP_AREA.each do |area, hash|
		if(hash[:map] == map &&
			hash[:a][:x] >= x && hash[:a][:y] >= y &&
			hash[:b][:x] <= x && hash[:b][:y] <= y)
			puts "Match: #{map}[#{x}, #{y}] = #{hash[:name]}"
			result = area
			count += 1
		end
	end
	# There should only be one result, but with this data, there will ocasionally be more.
	# We will need more accurate zone borders.
	puts "#{count} results."
	return result
end

# count of coords in zone
@zones = {}
@coords.each do |row|
	area = zoneFromCoords(row[:map], row[:position_x], row[:position_y])
	if(!@zones[area])
		@zones[area] = 1
	else
		@zones[area] += 1
	end
end

@zones.each do |zone, count|
	if(!@mainZone)
		@mainZone = zone
	elsif(@zones[@mainZone] < count)
		@mainZone = zone
	end
end

#puts "end"
STDOUT.flush
