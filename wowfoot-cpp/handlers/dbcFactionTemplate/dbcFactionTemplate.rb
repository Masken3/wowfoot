@singular = @dbcName = 'FactionTemplate'
@plural = 'FactionTemplates'
@upperCase = 'FACTION_TEMPLATES'

@id = 0
@struct = [
m(:int, 'nameId', 1),	# ref to Faction.dbc
]
