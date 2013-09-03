@singular = @dbcName = 'SpellDuration'
@plural = 'SpellDurations'
@upperCase = 'SPELL_DURATIONS'

@id = 0
@struct = [
# all measured in milliseconds.
m(:int, 'base', 1),
m(:int, 'perLevel', 2),
m(:int, 'max', 3),
]
