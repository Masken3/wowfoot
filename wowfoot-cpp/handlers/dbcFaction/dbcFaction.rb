@singular = @dbcName = 'Faction'
@plural = 'Factions'
@upperCase = 'FACTIONS'

@id = 0
@struct = [
m(:int, 'parent', 18),
m(:string, 'name', 19),
m(:string, 'description', 20 + STRING_LENGTH),
]
