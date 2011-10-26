@singular = @dbcName = 'WorldMapArea'
@plural = 'WorldMapAreas'
@upperCase = 'WORLD_MAP_AREA'

@id = 0
@struct = [
m(:int, 'map', 1),
m(:int, 'area', 2),
m(:string, 'name', 3),
f2('a', 6, 4),
f2('b', 7, 5),
]
