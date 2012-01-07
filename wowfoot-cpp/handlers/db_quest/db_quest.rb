@mysql_db_name = 'quest_template'
@structName = 'Quest'
@containerType = :map

@struct = [
c(:int, :entry, :key),
c(:int, :method),
c(:int, :zoneOrSort),
c(:int, :skillOrClassMask),
c(:int, :minLevel),
c(:int, :maxLevel),
c(:int, :questLevel),
c(:int, :type),
c(:int, :requiredRaces),
c(:int, :requiredSkillValue),
c(:string, :title),
c(:string, :details),
c(:string, :objectives),
c(:string, :offerRewardText),
c(:string, :requestItemsText),
c(:string, :endText),
c(:string, :completedText),
]

@index = [
]
