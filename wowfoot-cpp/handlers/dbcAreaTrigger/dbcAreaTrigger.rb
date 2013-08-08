@dbcName = @singular = 'AreaTrigger'
@plural = 'AreaTriggers'
@upperCase = 'AREA_TRIGGERS'

@id = 0
@struct = [
	m(:int, 'id', 0),
	m(:int, 'map', 1),
	m(:float, 'x', 2),
	m(:float, 'y', 3),
	m(:float, 'z', 4),
	m(:float, 'radius', 5),
	m(:float, 'box_x', 6),
	m(:float, 'box_y', 7),
	m(:float, 'box_z', 8),
	m(:float, 'box_orientation', 9),	# extent rotation about z axis
]
