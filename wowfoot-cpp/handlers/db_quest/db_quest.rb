@mysql_db_name = 'quest_template'
@structName = 'Quest'
@containerType = :map

@struct = [
c(:int, :id, :key),
c(:int, :method),
c(:int, :zoneOrSort),
c(:int, :minLevel),
c(:int, :maxLevel),
c(:int, :level),	# quest level
c(:int, :type),
c(:int, :requiredRaces),
c(:int, :requiredClasses),
c(:int, :requiredSkillId),
c(:int, :requiredSkillPoints),
c(:int, :requiredMinRepFaction),
c(:int, :requiredMinRepValue),
c(:int, :requiredMaxRepFaction),
c(:int, :requiredMaxRepValue),
# todo: prevId, nextId, group, chain
c(:string, :title),
c(:string, :details),
c(:string, :objectives),
c(:string, :offerRewardText),
c(:string, :requestItemsText),
c(:string, :endText),
c(:string, :completedText),
c(:int, :rewardOrRequiredMoney),
c(:int, :rewardMoneyMaxLevel),
c(:int, :rewardMailTemplateId),
c(:int, :rewardMailDelay),
mc(:int, [:rewardItemId, :rewardItemCount], 4),
mc(:int, [:rewardChoiceItemId, :rewardChoiceItemCount], 6),
mc(:int, [:rewardFactionId, :rewardFactionValueId, :rewardFactionValueIdOverride], 5),
c(:int, :rewardSpell),
c(:int, :rewardSpellCast),
]

@index = [
]
