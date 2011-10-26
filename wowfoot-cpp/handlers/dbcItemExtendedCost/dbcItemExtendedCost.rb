@singular = 'ItemExtendedCost'
@plural = 'ItemExtendedCosts'
@dbcName = 'ItemExtendedCost'
@upperCase = 'ITEM_EXTENDED_COST'

@id = 0
@struct = [
m(:int, 'honorPoints', 1),
m(:int, 'arenaPoints', 2),
m(:int, 'arenaSlot', 3),
m(:int, 'arenaRating', 14),
as('ReqItem', 'item', [m(:int, 'id', 4), m(:int, 'count', 9)], 5),
]
