@singular = @dbcName = 'Faction'
@plural = 'Factions'
@upperCase = 'FACTIONS'

@id = 0
@struct = [
m(:int, 'parent', 18),
m(:string, 'name', 23),
m(:string, 'description', 34),	# very uncertain number
]
