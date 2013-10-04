@singular = @dbcName = 'Talent'
@plural = 'Talents'
@upperCase = 'TALENT'

@id = 0
@struct = [
m(:int, 'id', 0),
m(:int, 'tabId', 1),	# TalentTab.dbc
m(:int, 'row', 2),
m(:int, 'col', 3),
ar(:int, 'spellId', 4, 5),	# Spell.dbc, up to 5 ranks.
m(:int, 'prereq', 13),	# Talent.dbc
m(:int, 'prereqRank', 16),
m(:int, 'prereqSpell', 20),	# always zero.
]
