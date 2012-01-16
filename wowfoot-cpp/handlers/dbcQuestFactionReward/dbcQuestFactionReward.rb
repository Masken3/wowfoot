@singular = @dbcName = 'QuestFactionReward'
@plural = 'QuestFactionRewards'
@upperCase = 'QUEST_FACTION_REWARD'

@id = 0
@struct = [
as('ReqItem', 'rep', [m(:int, 'value', 1)], 10),
]
