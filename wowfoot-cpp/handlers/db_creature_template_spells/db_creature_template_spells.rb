@mysql_db_name = 'creature_template_spells'
@structName = 'NpcSpell'

@struct = [
c(:int, :entry),
mc(:int, [:spell], 4)
]

@index = [
	[:spell],
]
