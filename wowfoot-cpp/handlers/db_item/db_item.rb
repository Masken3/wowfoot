@mysql_db_name = 'item_template'
@structName = 'Item'

@struct = [
c(:int, :entry),
c(:int, :class),
c(:int, :subclass),
c(:string, :name),
c(:int, :displayId),
c(:int, :quality),
c(:int, :flags),
]
@struct << c(:int, :flagsExtra) if(CONFIG_WOW_VERSION > 20000)
@struct += [
c(:int, :buyCount),
c(:int, :buyPrice),
c(:int, :sellPrice),
c(:int, :inventoryType),
c(:int, :allowableClass),
c(:int, :allowableRace),
c(:int, :itemLevel),
c(:int, :requiredLevel),
c(:int, :requiredSkill),
c(:int, :requiredSkillRank),
c(:int, :maxCount),
c(:int, :stackable),
c(:int, :containerSlots),
]
@struct << c(:int, :statsCount) if(CONFIG_WOW_VERSION > 20000)
@struct << mc(:int, [:stat_type, :stat_value], 10)
if(CONFIG_WOW_VERSION < 20000)
	@struct << mc(:float, [:dmg_min, :dmg_max], 5)
	@struct << mc(:int, [:dmg_type], 5)
else
	@struct << mc(:float, [:dmg_min, :dmg_max], 2)
	@struct << mc(:int, [:dmg_type], 2)
end
@struct += [
c(:int, :armor),
c(:int, :holy_res),
c(:int, :fire_res),
c(:int, :nature_res),
c(:int, :frost_res),
c(:int, :shadow_res),
c(:int, :arcane_res),
c(:int, :delay),
c(:int, :ammo_type),
c(:int, :bonding),
c(:string, :description),
c(:int, :startQuest),
c(:int, :lockId),
c(:int, :material),
c(:int, :sheath),
c(:int, :randomProperty),
]
c(:int, :randomSuffix) if(CONFIG_WOW_VERSION > 20000)
@struct += [
c(:int, :block),
c(:int, :itemset),
c(:int, :maxDurability),
c(:int, :bagFamily),
]
c(:int, :totemCategory) if(CONFIG_WOW_VERSION > 20000)
c(:int, :requiredDisenchantSkill) if(CONFIG_WOW_VERSION > 20000)
@struct += [
c(:int, :duration),
c(:int, :disenchantId),
c(:int, :foodType),
c(:int, :minMoneyLoot),
c(:int, :maxMoneyLoot),
mc(:int, [:spellId, :spellTrigger, :spellCharges, :spellCooldown,
	:spellCategory, :spellCategoryCooldown], 5, '_'),
mc(:float, [:spellPpmRate], 5, '_'),
]

@index = [
[:spellId],
[:displayId],
[:class],
[:class, :subclass],
[:itemset],
[:startQuest],
[:lockId],
]
