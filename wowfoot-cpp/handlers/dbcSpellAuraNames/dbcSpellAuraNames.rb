@singular = 'SpellAuraName'
@plural = @dbcName = 'SpellAuraNames'
@upperCase = 'SPELL_AURA_NAMES'

@id = 0
@struct = [
m(:int, 'category', 1),
m(:string, 'internalName', 2),
m(:string, 'name', 3),
]
