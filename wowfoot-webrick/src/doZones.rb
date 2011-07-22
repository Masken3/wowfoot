
# count of coords in zone
@zones = {}
@coords.each do |row|
	map = row[:map]
	x, y = row[:position_x], row[:position_y]
	zoneId, areaId = zoneFromCoords(map, x, y)
	if(!zoneId)
		#if(!@zones[areaId])
		#	@zones[areaId] = 0
		#else
		#	@zones[areaId] += 1
		#end
		# todo: handle instance maps better.
		@zones[areaId] = false if(AREA_TABLE[areaId])
		next
	end
	#puts "Area: #{area}"
	if(!@zones[zoneId])
		#puts "Setup: #{area}"
		@zones[zoneId] = []
	end
	@zones[zoneId] << percentagesInZone(zoneId, x, y)
end

@mainArea = nil
@zones.each do |area, coords|
	#p area, coords
	#p @mainArea
	#p @zones[@mainArea]
	if(!@mainArea)
		@mainArea = area
	elsif(coords && @zones[@mainArea].size < coords.size)
		@mainArea = area
	end
end

if(@mainArea && WORLD_MAP_CONTINENT[AREA_TABLE[@mainArea][:map]])
	@mapImageName = WORLD_MAP_AREA[@mainArea][:name]
else
	@mapImageName = nil
end

def map
	if(@mainArea)
		html = "#{AREA_TABLE[@mainArea][:name]}<br>\n"
	else
		html = ''
	end
	if(@mapImageName)
		html += "<img width=\"100%\" src=\"output/#{@mapImageName}.jpeg\">\n"
	else
		html += "No map available.<br>\n"
		if(@coords && @coords[0])
			row = @coords[0]
			mapId = row[:map]
			x, y = row[:position_x].to_i, row[:position_y].to_i
			html += "Map #{mapId} (#{MAP[mapId]}), [#{x}][#{y}]<br>\n"
		end
	end
	return html
end