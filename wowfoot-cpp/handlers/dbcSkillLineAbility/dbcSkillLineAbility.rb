@singular = @dbcName = 'SkillLineAbility'
@plural = 'SkillLineAbilities'
@upperCase = 'SKILLLINEABILITIES'

@id = 0
@struct = [
	m(:int, 'skill', 1),
	m(:int, 'spell', 2),
	m(:int, 'racemask', 3),
	m(:int, 'classmask', 4),
	m(:int, 'reqSkillValue', 7),
	m(:int, 'forwardSpell', 8),
	m(:int, 'learnOnGetSkill', 9),
	m(:int, 'maxValue', 10),
	m(:int, 'minValue', 11),
]
