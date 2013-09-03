@singular = @dbcName = 'SpellRange'
@plural = 'SpellRanges'
@upperCase = 'SPELL_RANGES'

@id = 0
@struct = [
# measured in yards.
m(:float, 'min', 1),
m(:float, 'max', 2),
]
