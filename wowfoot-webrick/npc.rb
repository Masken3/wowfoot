
# todo: prepare statements in advance.
stm = TDB::C.prepare('select entry, name, subname, minlevel, maxlevel, rank'+
	' from creature_template where entry = ?')
stm.execute(@id)
@template = stm.fetch

stm = TDB::C.prepare('select guid, id, map, position_x, position_y, position_z'+
	' from creature where id = ?')
stm.execute(@id)
@coords = stm.fetch_all

#puts "end"
STDOUT.flush
