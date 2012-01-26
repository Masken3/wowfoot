@singular = @dbcName = 'ItemSet'
@plural = 'ItemSets'
@upperCase = 'ITEM_SETS'

@id = 0
@struct = [
m(:int, 'id', 0),
m(:string, 'name', 1),
ar(:int, 'item', 18, 10),
as('Spell', 'spell', [m(:int, 'id', 35), m(:int, 'req', 43)], 8),
]
