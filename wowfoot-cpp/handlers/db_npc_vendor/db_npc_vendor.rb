@mysql_db_name = 'npc_vendor'
@structName = 'NpcVendor'
@containerType = :set

@struct = [
	[:int, :entry, :key],
	[:int, :slot],
	[:int, :item, :key],
	[:int, :maxcount],
	[:int, :extendedCost, :key],
]

@index = [
	[:item],
]
