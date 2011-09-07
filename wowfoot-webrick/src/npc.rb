
# todo: prepare statements in advance.
stm = TDB::C.prepare('select entry, name, subname, minlevel, maxlevel, rank'+
	' from creature_template where entry = ?')
stm.execute(@id)
@template = stm.fetch

raise HTTPStatus[404], "NPC not found in database" if(!@template)

stm = TDB::C.prepare('select guid, map, position_x, position_y, position_z'+
	' from creature where id = ?')
stm.execute(@id)
@coords = stm.fetch_all

run 'doZones.rb'

@commentTable = 'npc'
run 'comments.rb'
