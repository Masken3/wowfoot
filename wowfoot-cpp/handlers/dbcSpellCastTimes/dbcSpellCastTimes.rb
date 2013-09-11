@singular = 'SpellCastTime'
@plural = @dbcName = 'SpellCastTimes'
@upperCase = 'SPELL_CAST_TIMES'

@id = 0
@struct = [
# all measured in milliseconds.
m(:int, 'base', 1),
]
