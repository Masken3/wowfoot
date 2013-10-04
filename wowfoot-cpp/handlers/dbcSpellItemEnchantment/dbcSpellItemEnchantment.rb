@singular = @dbcName = 'SpellItemEnchantment'
@plural = 'SpellItemEnchantments'
@upperCase = 'SPELL_ITEM_ENCHANTMENTS'

effectMembers = [
m(:int, 'type', 1),
m(:int, 'amount', 4),
m(:int, 'spellId', 10),
]

@id = 0
@struct = [
as('Effect', 'effect', effectMembers, 3),
m(:string, 'description', 13),
m(:int, 'auraId', 22),
m(:int, 'flags', 23),
]
