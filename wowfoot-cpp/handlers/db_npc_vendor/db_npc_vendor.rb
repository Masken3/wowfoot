@mysql_db_name = 'npc_vendor'
@structName = 'NpcVendor'
@containerType = :set

@struct = [
c(:int, :entry, :key),
c(:int, :slot),
c(:int, :item, :key),
c(:int, :maxcount),
c(:int, :extendedCost, :key),
]

@index = [
	[:item],
	[:extendedCost],
]
