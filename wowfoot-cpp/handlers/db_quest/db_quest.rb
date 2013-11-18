@mysql_db_name = 'quest_template'
@structName = 'Quest'
@containerType = :map

def cReward(subName); ('reward' + subName).to_sym; end

if(CONFIG_WOW_VERSION > 30000)
	@struct = [
		c(:int, :id, :key),
		c(:int, :maxLevel),
		c(:int, :level),	# quest level
		c(:int, :requiredSkillId),
		c(:int, :requiredSkillPoints),
		c(:string, :completedText),
		c(:int, :rewardOrRequiredMoney),
		renamed(:int, :rewardMailDelay, :rewardMailDelaySecs),
	]
	def reward(subName); cReward(subName); end
else
	@struct = [
		renamed(:int, :entry, :id, :key),
		renamed(:int, :questLevel, :level),
		renamed(:int, :requiredSkill, :requiredSkillId),
		renamed(:int, :requiredSkillValue, :requiredSkillPoints),
		renamed(:int, :rewOrReqMoney, :rewardOrRequiredMoney),
		renamed(:int, :rewMailDelaySecs, :rewardMailDelaySecs),
	]
	def reward(subName); ('rew' + subName).to_sym; end

	alias :oldEscape :cEscape
	def cEscape(name)
		ns = name.to_s
		if(ns.start_with?('rew') && !ns.start_with?('reward'))
			return ('reward' + ns[3..-1]).to_sym
		else
			return oldEscape(name)
		end
	end
end
def r(subName); renamed(:int, reward(subName), cReward(subName)); end
def mr(subNames, count)
	mc(:int, (subNames.collect do |sn | reward(sn); end), count)
end

@struct += [
c(:int, :method),
c(:int, :zoneOrSort),
c(:int, :minLevel),
]
@struct += [
c(:int, :type),
c(:int, :requiredRaces),
c(:int, :requiredClasses),
c(:int, :repObjectiveFaction),
c(:int, :repObjectiveValue),
c(:int, :requiredMinRepFaction),
c(:int, :requiredMinRepValue),
c(:int, :requiredMaxRepFaction),
c(:int, :requiredMaxRepValue),
c(:int, :prevQuestId),
c(:int, :nextQuestId),
c(:int, :exclusiveGroup),
c(:int, :nextQuestInChain),
# todo: prevId, nextId, group, chain
c(:string, :title),
c(:string, :details),
c(:string, :objectives),
c(:string, :offerRewardText),
c(:string, :requestItemsText),
c(:string, :endText),
renamed(:struct, 'Objective', 'objective', [
	renamed(:string, :objectiveText, :text),
	c(:int, :reqItemId), c(:int, :reqItemCount),
	c(:int, :reqSourceId), c(:int, :reqSourceCount),
	c(:int, :reqCreatureOrGOId), c(:int, :reqCreatureOrGOCount),
	c(:int, :reqSpellCast),
	], 4),
r('MoneyMaxLevel'),
r('MailTemplateId'),
mr(['ItemId', 'ItemCount'], 4),
mr(['ChoiceItemId', 'ChoiceItemCount'], 6),
r('Spell'),
r('SpellCast'),
c(:int, :srcItemId),
c(:int, :srcSpell),
]

@index = [
	[:zoneOrSort],
	[:srcItemId],	# given at start, auto-deleted at abandon.
	[:srcSpell],	# cast on player at start.
	[:reqSpellCast],	# objective: cast this spell.
	[:reqItemId],	# objective: get Count of these items.
	[:reqSourceId],	# quest item, not an objetive by itself, but is used to finish the objective.
	[:reqCreatureOrGOId],	# objective: positive npcId or negative gameobjectId.
	[:rewItemId],	# reward: you will get Count of these items.
	[:rewChoiceItemId],	# reward: choose one.
	[:rewardSpell],
	[:rewardSpellCast],
	[:prevQuestId],
	[:nextQuestId],
	[:nextQuestInChain],
	[:exclusiveGroup],
]

if(CONFIG_WOW_VERSION > 30000)
	@struct << mc(:int, [:rewardFactionId, :rewardFactionValueId, :rewardFactionValueIdOverride], 5)
	@index << [:rewardFactionId]
else
	@struct << mc(:int, [:rewRepFaction, :rewRepValue], 5)
	@index << [:rewRepFaction]
end
