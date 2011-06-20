
# todo: prepare statements in advance.
stm = TDB::C.prepare('select entry, name, subname, minlevel, maxlevel, rank'+
	' from creature_template where entry = ?')
stm.execute(@id)
@template = stm.fetch

stm = TDB::C.prepare('select guid, id, map, position_x, position_y, position_z'+
	' from creature where id = ?')
stm.execute(@id)
@coords = stm.fetch_all

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
		@zones[areaId] = false
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
