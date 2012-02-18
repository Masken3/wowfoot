@mysql_db_name = 'creature_onkill_reputation'
@structName = 'NpcRep'

@struct = [
c(:int, :creature_id),
mc(:int, [:rewOnKillRepFaction, :maxStanding, :isTeamAward, :rewOnKillRepValue], 2),
c(:int, :teamDependent),
]

@index = [
	[:rewOnKillRepFaction],
]
