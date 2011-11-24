@mysql_db_name = ''
@structName = 'Spawn'
@containerType = :set

@prefixes = [
	'creature',
	'gameobject',
]

@struct = [
c(:int, :guid, :key),
c(:int, :id),
c(:int, :map),
c(:float, :position_x),
c(:float, :position_y),
c(:float, :position_z),
]

@index = [
	[:id],
]
