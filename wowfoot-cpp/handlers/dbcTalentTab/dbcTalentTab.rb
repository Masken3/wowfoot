@singular = @dbcName = 'TalentTab'
@plural = 'TalentTabs'
@upperCase = 'TALENT_TAB'

@id = 0
@struct = [
m(:int, 'id', 0),
m(:string, 'name', 1),
m(:int, 'spellIcon', 10),
#m(:int, 'raceMask', 11),	# irrelevant; all races have the same talents.
m(:int, 'classMask', 12),
m(:int, 'tabPage', 13),
m(:string, 'internalName', 14),
]
