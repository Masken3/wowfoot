@mysql_db_name = 'achievement_reward'
@structName = 'AchievementReward'

@struct = [
c(:int, :entry),
c(:int, :title_A),
c(:int, :title_H),
c(:int, :item),
c(:int, :sender),
c(:string, :subject),
c(:string, :text),
]

@index = [
	[:title_A],
	[:title_H],
	[:item],
	[:sender],
]
