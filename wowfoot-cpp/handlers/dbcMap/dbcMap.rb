@singular = @dbcName = 'Map'
@plural = 'Maps'
@upperCase = 'MAPS'

@id = 0
@struct = [
m(:string, 'internalName', 1),
m(:int, 'type', 2),
m(:string, 'name', 4),
m(:int, 'linkedZone', 19),
]
