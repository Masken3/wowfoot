@singular = @dbcName = 'SkillLine'
@plural = 'SkillLines'
@upperCase = 'SKILL_LINES'

@id = 0
@struct = [
	m(:int, 'id', 0),
	m(:int, 'category', 1),	# id into SkillLineCategory
	m(:int, 'skillCost', 2),
	m(:string, 'name', 3),
]
if(CONFIG_WOW_VERSION < 20000)
	@struct << m(:string, 'description', 12)
	@struct << m(:int, 'spellIcon', 21)
else
	@struct << m(:string, 'description', 20)
	@struct << m(:int, 'spellIcon', 37)
	@struct << m(:string, 'alternateVerb', 38)
	@struct << m(:int, 'canLink', 55)
end
