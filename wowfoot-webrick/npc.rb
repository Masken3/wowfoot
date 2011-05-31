
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
	percentages = {}
	count = 0
	WORLD_MAP_AREA.each do |area, hash|
		if(hash[:map] == map &&
			hash[:a][:x] >= x && hash[:a][:y] >= y &&
			hash[:b][:x] <= x && hash[:b][:y] <= y)
			puts "Match: #{map}[#{x}, #{y}] = #{area} #{hash[:name]}"
			result = area
			count += 1
			width = hash[:a][:x] - hash[:b][:x]
			percentages[:x] = 1.0 - ((x - hash[:b][:x]) / width)
			height = hash[:a][:y] - hash[:b][:y]
			percentages[:y] = 1.0 - ((y - hash[:b][:y]) / height)
			#return result, percentages
		end
	end
	# There should only be one result, but with this data, there will ocasionally be more.
	# We will need more accurate zone borders.
	STDOUT.flush
	#raise "#{count} results." if(count > 1)
	return result, percentages
end

# unused
def Compute(x, y, center_offset, size)
	x_offset = (x - center_offset)/size
	y_offset = (y - center_offset)/size
	
	x_val = x_offset + CENTER_VAL + 0.5
	y_val = y_offset + CENTER_VAL + 0.5
	return x_val, y_val
end

def ComputeGridPair(x, y)
	return Compute(x, y, CENTER_GRID_OFFSET, SIZE_OF_GRIDS)
end


# count of coords in zone
@zones = {}
@coords.each do |row|
	area, coords = zoneFromCoords(row[:map], row[:position_x], row[:position_y])
	puts "Area: #{area}"
	if(!@zones[area])
		puts "Setup: #{area}"
		@zones[area] = []
	end
	@zones[area] << coords
end

@mainArea = nil
@zones.each do |area, coords|
	#p area, coords
	p @mainArea
	p @zones[@mainArea]
	if(!@mainArea)
		@mainArea = area
	elsif(@zones[@mainArea].size < coords.size)
		@mainArea = area
	end
end

#puts "end"
STDOUT.flush
