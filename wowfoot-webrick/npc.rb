
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
	area, coords = zoneFromCoords(row[:map], row[:position_x], row[:position_y])
	#puts "Area: #{area}"
	if(!@zones[area])
		#puts "Setup: #{area}"
		@zones[area] = []
	end
	@zones[area] << coords
end

@mainArea = nil
@zones.each do |area, coords|
	#p area, coords
	#p @mainArea
	#p @zones[@mainArea]
	if(!@mainArea)
		@mainArea = area
	elsif(@zones[@mainArea].size < coords.size)
		@mainArea = area
	end
end

#puts "end"
STDOUT.flush
