@mysql_db_name = 'npc_vendor'
@structName = 'NpcVendor'

@struct = [
	[:int, :entry],
	[:int, :slot],
	[:int, :item],
	[:int, :maxcount],
	[:int, :extendedCost],
]

@index = [
	[:item],
]
