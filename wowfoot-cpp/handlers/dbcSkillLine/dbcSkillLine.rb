@singular = @dbcName = 'SkillLine'
@plural = 'SkillLines'
@upperCase = 'SKILL_LINES'

@id = 0
@struct = [
	m(:int, 'category', 1),	# id into SkillLineCategory
	m(:int, 'skillCost', 2),
	m(:string, 'name', 3),
	m(:string, 'description', 20),
	m(:int, 'spellIcon', 37),
	m(:string, 'alternateVerb', 38),
	m(:int, 'canLink', 55),
]
