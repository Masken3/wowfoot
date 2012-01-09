@singular = @dbcName = 'Achievement'
@plural = 'Achievements'
@upperCase = 'ACHIEVEMENTS'

@id = 0
@struct = [
m(:int, 'faction', 1),
m(:int, 'map', 2),
m(:int, 'previous', 3),
m(:string, 'name', 4),
m(:string, 'description', 21),
m(:int, 'points', 39),
m(:string, 'reward', 43),
]
