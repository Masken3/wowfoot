
combos = [
	['ReqItem', 6],
	['ReqSource', 4],
	['ReqCreatureOrGO', 4],
	['RewChoiceItem', 6],
	['ReqItem', 4],
]

arrays = [
	['RewRepFaction', 5],
	['RewRepValueId', 5],
	['RewRepValue', 5],
	['ReqSpellCast', 4],
]

comboSelect = ''
combos.each do |c|
	(1..c[1]).each do |i|
		comboSelect += ", #{c[0]}Id#{i}, #{c[0]}Count#{i}"
	end
end

arraySelect = ''
arrays.each do |c|
	(1..c[1]).each do |i|
		arraySelect += ", #{c[0]}#{i}"
	end
end

# todo: prepare statements in advance.
stm = TDB::C.prepare('select entry, zoneOrSort, minlevel, maxlevel, questlevel'+
	', requiredRaces, prevQuestId, nextQuestId, nextQuestInChain, exclusiveGroup, title, details'+
	', objectives, offerRewardText, requestItemsText, completedText'+
	', rewOrReqMoney, rewMoneyMaxLevel, rewMailTemplateId, rewMailDelaySecs'+
	comboSelect + arraySelect +
	' from quest_template where entry = ?')
stm.execute(@id)
@template = stm.fetch
def translateQuestText(sym)
	bPattern = /\$[bB]/
	@template[sym].gsub!(bPattern, '<br>') if(@template[sym])
end
translateQuestText(:objectives)
translateQuestText(:details)
translateQuestText(:offerRewardText)
translateQuestText(:requestItemsText)
translateQuestText(:completedText)

questLevel = @template[:questlevel]
rewMoneyMaxLevel = @template[:rewMoneyMaxLevel]
if(questLevel >= 65)
	@rewXp = rewMoneyMaxLevel / 6
elsif(questLevel == 64)
	@rewXp = rewMoneyMaxLevel / 4.8
elsif(questLevel == 63)
	@rewXp = rewMoneyMaxLevel / 3.6
elsif(questLevel == 62)
	@rewXp = rewMoneyMaxLevel / 2.4
elsif(questLevel == 61)
	@rewXp = rewMoneyMaxLevel / 1.2
else	#if(questLevel <= 60)
	@rewXp = rewMoneyMaxLevel / 0.6
end

zoneOrSort = @template[:zoneOrSort]
if(zoneOrSort < 0)
	@sort = -zoneOrSort
	@zone = nil
else
	@sort = nil
	@zone = zoneOrSort
end

def fetchQuest(stm, id)
	return nil if(id == 0)
	stm.execute(id)
	return stm.fetch
end

@prevQuest = fetchQuest(stm, @template[:prevQuestId])
@exclusiveGroup = fetchQuest(stm, @template[:exclusiveGroup])
@nextQuest = fetchQuest(stm, @template[:nextQuestId])
@nextQuestInChain = fetchQuest(stm, @template[:nextQuestInChain])
if(!@prevQuest)
	stm = TDB::C.prepare('select entry, zoneOrSort, minlevel, maxlevel, questlevel'+
		', requiredRaces, prevQuestId, nextQuestId, nextQuestInChain, title, details'+
		', objectives, offerRewardText, requestItemsText, completedText'+
		' from quest_template where (nextQuestId = ?) AND (entry != ?)')
	stm.execute(@id, @id)
	@prevQuest = stm.fetch_all
else
	@prevQuest = [@prevQuest]
end

stm = TDB::C.prepare('select id, name'+
	' from creature_questrelation'+
	' INNER JOIN creature_template ON creature_template.entry = creature_questrelation.id'+
	' where creature_questrelation.quest = ?')
stm.execute(@id)
@givers = stm.fetch_all

stm = TDB::C.prepare('select id, name'+
	' from creature_involvedrelation'+
	' INNER JOIN creature_template ON creature_template.entry = creature_involvedrelation.id'+
	' where creature_involvedrelation.quest = ?')
stm.execute(@id)
@finishers = stm.fetch_all

stm = CDB::C.prepare('select user, body, body as originalBody, rating, date, indent'+
	' from comments'+
	' INNER JOIN quest_comments on commentId = id'+
	' where quest_comments.entry = ?')
#stm = CDB::C.prepare('select commentId from quest_comments where entry = ?')
stm.execute(@id)
@comments = stm.fetch_all

def subSimpleTag(body, tag)
	body.gsub!("[#{tag}]", "<#{tag}>")
	body.gsub!("[/#{tag}]", "</#{tag}>")
end

@comments.each do |c|
	b = c[:body]
	#c[:originalBody] = b
	b.gsub!('\\n', "<br>\n")
	subSimpleTag(b, 'b')
	subSimpleTag(b, 'i')
	subSimpleTag(b, 'u')
	subSimpleTag(b, 'ul')
	subSimpleTag(b, 'li')
	#subSimpleTag(b, 'quote')	# doesn't work; quote tags are complex.
	
	# doesn't work.
	#c[:body].gsub!(/\[url=http:\/\/.+\.wowhead\.com\/\?([^]]+)\]/, '<a href="/\1">')
	
	# let's do it the simple & long way instead.
	i = 0
	urlSpec = '[url='
	while(i = b.index(urlSpec, i))
		endIndex = b.index(']', i)
		urlStartIndex = i + urlSpec.length
		url = b[urlStartIndex, endIndex - urlStartIndex]
		url.gsub!(/http:\/\/.+\.wowhead\.com\/\?(.+)/, '/\1')
		url.gsub!(/http:\/\/.+\.wowhead\.com\/(.+)/, '/\1')
		endIndex += 1
		sub = "<a href=\"#{url}\">"
		b[i, endIndex - i] = sub
		i += sub.length
	end
	b.gsub!('[/url]', '</a>')
	i = 0
	# example: quest=2604
	# todo: look them up in the database, get names.
	while(i = b.index('[', i))
		endIndex = b.index(']', i)
		if(!endIndex)
			i += 1
			next
		end
		path = b[i+1, (endIndex - i) - 1]
		endIndex += 1
		sub = "<a href=\"/#{path}\">#{path}</a>"
		b[i, endIndex - i] = sub
		i += sub.length
	end
	c[:body] = b
end
