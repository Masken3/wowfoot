@singular = @dbcName = 'FactionTemplate'
@plural = 'FactionTemplates'
@upperCase = 'FACTION_TEMPLATES'

@id = 0
@struct = [
m(:int, 'nameId', 1),	# ref to Faction.dbc
m(:int, 'flags', 2),
m(:int, 'ourMask', 3),
m(:int, 'friendlyMask', 4),
m(:int, 'hostileMask', 5),
ar(:int, 'enemyFaction', 6, 4),
ar(:int, 'friendFaction', 10, 4),
]
