@singular = @dbcName = 'ItemSet'
@plural = 'ItemSets'
@upperCase = 'ITEM_SETS'

if(CONFIG_WOW_VERSION > 20000)
	off = 8
else
	off = 0
end

@id = 0
@struct = [
m(:int, 'id', 0),
m(:string, 'name', 1),
ar(:int, 'item', 10+off, 10),
as('Spell', 'spell', [m(:int, 'id', 27+off), m(:int, 'req', 35+off)], 8),
]
