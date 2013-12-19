@singular = @dbcName = 'Spell'
@plural = 'Spells'
@upperCase = 'SPELLS'

def size(type)
	case type
		when :int; return 1;
		when :string; return @stringSize;
		when :float; return 1;
		else; raise "Don't know size for type: #{type}"
	end
end

EFFECT_COUNT = 3

EffectMember = Struct.new(:type, :name)
def e(type, name); EffectMember.new(type, name); end
def convertEffectMember(c)
	pos = @memberPos
	@memberPos += EFFECT_COUNT * size(c.type)
	return m(c.type, c.name, pos)
end
def f(type, name)
	pos = @memberPos
	@memberPos += size(type)
	return m(type, name, pos)
end
def fa(type, name, count)
	pos = @memberPos
	@memberPos += count * size(type)
	return ar(type, name, pos, count)
end


if(CONFIG_WOW_VERSION > 20000)
	@basePos = 40
	@memberPos = 71
	@stringSize = 16
	effectMembersA = []
	effectMembersB = [
		e(:int, 'miscValueB'),
	]
	effectMembersC = [
		e(:int, 'spellClassMaskA'),
		e(:int, 'spellClassMaskB'),
		e(:int, 'spellClassMaskC'),
	]
else
	@basePos = 30
	@memberPos = 61
	@stringSize = 8
	effectMembersA = [
		e(:int, 'baseDice'),
		e(:float, 'dicePerLevel'),
	]
	effectMembersB = []
	effectMembersC = []
end

effectMembers = [
	e(:int, 'id'),
	e(:int, 'dieSides'),
] + effectMembersA + [
	e(:float, 'realPointsPerLevel'),
	e(:int, 'basePoints'),
	e(:int, 'mechanic'),
	e(:int, 'implicitTargetA'),
	e(:int, 'implicitTargetB'),
	e(:int, 'radiusIndex'),
	e(:int, 'applyAuraName'),
	e(:int, 'amplitude'),
	e(:float, 'multipleValue'),
	e(:int, 'chainTarget'),
	e(:int, 'itemType'),
	e(:int, 'miscValue'),
] + effectMembersB + [
	e(:int, 'triggerSpell'),
	e(:float, 'pointsPerComboPoint'),
] + effectMembersC

effectMembers = effectMembers.collect do |c|
	convertEffectMember(c)
end

effectMembers += [
	m(:float, 'DmgMultiplier', 167)
]

plainMembers = []
@memberPos = 1
if(CONFIG_WOW_VERSION < 20000)
	plainMembers << f(:int, 'School')
end
plainMembers << f(:int, 'Category')
if(CONFIG_WOW_VERSION < 20000)
	plainMembers << f(:int, 'castUI')
end
plainMembers += [
	f(:int, 'Dispel'),
	f(:int, 'Mechanic'),
	f(:int, 'Attributes'),
	f(:int, 'AttributesEx'),
	f(:int, 'AttributesEx2'),
	f(:int, 'AttributesEx3'),
	f(:int, 'AttributesEx4'),
]
if(CONFIG_WOW_VERSION > 20000)
	plainMembers += [
	f(:int, 'AttributesEx5'),
	f(:int, 'AttributesEx6'),
	f(:int, 'AttributesEx7'),
	]
end
plainMembers << f(:int, 'Stances')
plainMembers << f(:int, 'unk_320_2') if(CONFIG_WOW_VERSION > 32000)
plainMembers << f(:int, 'StancesNot')
plainMembers << f(:int, 'unk_320_3') if(CONFIG_WOW_VERSION > 32000)
plainMembers += [
	f(:int, 'Targets'),
	f(:int, 'TargetCreatureType'),
	f(:int, 'RequiresSpellFocus'),
]
f(:int, 'FacingCasterFlags') if(CONFIG_WOW_VERSION > 20000)
plainMembers += [
	f(:int, 'CasterAuraState'),
	f(:int, 'TargetAuraState'),
]
if(CONFIG_WOW_VERSION > 20000)
	plainMembers += [
	f(:int, 'CasterAuraStateNot'),
	f(:int, 'TargetAuraStateNot'),
	f(:int, 'casterAuraSpell'),
	f(:int, 'targetAuraSpell'),
	f(:int, 'excludeCasterAuraSpell'),
	f(:int, 'excludeTargetAuraSpell'),
	]
end
plainMembers += [
	f(:int, 'CastingTimeIndex'),
	f(:int, 'RecoveryTime'),
	f(:int, 'CategoryRecoveryTime'),
	f(:int, 'InterruptFlags'),
	f(:int, 'AuraInterruptFlags'),
	f(:int, 'ChannelInterruptFlags'),
	f(:int, 'procFlags'),
	f(:int, 'procChance'),
	f(:int, 'procCharges'),
	f(:int, 'maxLevel'),
	f(:int, 'baseLevel'),
	f(:int, 'spellLevel'),
	f(:int, 'DurationIndex'),
	f(:int, 'powerType'),
	f(:int, 'manaCost'),
	f(:int, 'manaCostPerLevel'),
	f(:int, 'manaPerSecond'),
	f(:int, 'manaPerSecondPerLevel'),
	f(:int, 'rangeIndex'),
	f(:float, 'speed'),
	f(:int, 'modalNextSpell'),
	f(:int, 'StackAmount'),
	fa(:int, 'Totem', 2),
]
@memberPos += 16
plainMembers += [
	f(:int , 'EquippedItemClass'),
	f(:int , 'EquippedItemSubClassMask'),
	f(:int , 'EquippedItemInventoryTypeMask'),
]
if(CONFIG_WOW_VERSION > 20000)
	@memberPos = 131
else
	@memberPos = 115
end
plainMembers += [
	fa(:int, 'spellVisual', 2),
	f(:int, 'spellIconID'),
	f(:int, 'activeIconID'),
	f(:int, 'spellPriority'),
	f(:string, 'name'),
	f(:int, 'SpellNameFlag'),
	f(:string, 'rank'),
	f(:int, 'RankFlags'),
	f(:string, 'description'),
	f(:int, 'DescriptionFlags'),
	f(:string, 'toolTip'),
	f(:int, 'ToolTipFlags'),
	f(:int, 'ManaCostPercentage'),
	f(:int, 'StartRecoveryCategory'),
	f(:int, 'StartRecoveryTime'),
	f(:int, 'MaxTargetLevel'),
	f(:int, 'SpellFamilyName'),
	f(:int, 'SpellFamilyFlags'),
	f(:int, 'SpellFamilyFlags2'),
	f(:int, 'MaxAffectedTargets'),
	f(:int, 'DmgClass'),
	f(:int, 'PreventionType'),
	f(:int, 'StanceBarOrder'),
	f(:int, 'MinFactionId'),
	f(:int, 'MinReputation'),
	f(:int, 'RequiredAuraVision'),
]
if(CONFIG_WOW_VERSION > 20000)
	plainMembers += [
	fa(:int, 'TotemCategory', 2),
	f(:int , 'AreaGroupId'),
	f(:int, 'SchoolMask'),
	f(:int, 'runeCostID'),
	f(:int, 'spellMissileID'),
	f(:int, 'PowerDisplayId'),
	fa(:float, 'unk_320_4', 3),	# probably effect-related.
	f(:int, 'spellDescriptionVariableID'),
	f(:int, 'SpellDifficultyId'),
	]
end

@id = 0
@struct = [
m(:int, 'id', 0),
as('Reagent', 'reagent', [m(:int, 'id', @basePos + 12), m(:int, 'count', @basePos + 20)], 8),
as('Effect', 'effect', effectMembers, 3),
] + plainMembers

@endOfHeader = "#define SPELL_EFFECT_MEMBERS(m)\\\n"
effectMembers.each do |em|
	@endOfHeader << "\tm(#{em[1]})\\\n"
end
@endOfHeader << "\n"

@endOfHeader << "#define SPELL_PLAIN_MEMBERS(m)\\\n"
plainMembers.each do |em|
	@endOfHeader << "\tm(#{em[1]})\\\n"
end
@endOfHeader << "\n"
