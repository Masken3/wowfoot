@mysql_db_name = 'creature'
@structName = 'NpcSpawn'
@containerType = :set

@struct = [
	[:int, :guid, :key],
	[:int, :id],
	[:int, :map],
	[:float, :position_x],
	[:float, :position_y],
	[:float, :position_z],
]
