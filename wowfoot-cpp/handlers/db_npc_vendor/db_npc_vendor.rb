@mysql_db_name = 'npc_vendor'
@structName = 'NpcVendor'
@containerType = :set

@struct = [
c(:int, :entry, :key),
c(:int, :item, :key),
c(:int, :maxcount),
]

@index = [
	[:item],
	[:entry],
]

if(CONFIG_WOW_VERSION > 20000)
	@struct << c(:int, :extendedCost, :key)
	@index << :extendedCost
end
