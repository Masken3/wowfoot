run 'coordinates.rb'

# todo: prepare statements in advance.
stm = TDB::C.prepare('select entry, name, type, displayId'+
	' from gameobject_template where entry = ?')
stm.execute(@id)
@template = stm.fetch

raise HTTPStatus[404], "Object not found in database" if(!@template)

stm = TDB::C.prepare('select guid, map, position_x, position_y, position_z'+
	' from gameobject where id = ?')
stm.execute(@id)
@coords = stm.fetch_all

run 'doZones.rb'
