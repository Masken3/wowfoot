
rewCombos = [
	{:id => :rewChoiceItem, :count => 6, :type => :item},
	{:id => :rewItem, :count => 4, :type => :item},
]
reqCombos = [
	{:id => :reqItem, :count => 6, :type => :item},
	{:id => :reqSource, :count => 4, :type => :item},
	{:id => :reqCreatureOrGO, :count => 4, :type => :co},
]

combos = rewCombos + reqCombos

arrays = [
	{:id => :rewRepFaction, :count => 5, :type => :faction},
	{:id => :rewRepValueId, :count => 5},
	{:id => :rewRepValue, :count => 5},
	{:id => :reqSpellCast, :count => 4, :type => :spell},
]

def selectComboSql(type, id)
	tt = "#{type}_template"
	return "(select name from #{tt} where #{tt}.entry = #{id})"
end

comboSelect = ''
comboJoin = ''
combos.each do |c|
	(1..c[:count]).each do |i|
		id = "#{c[:id]}Id#{i}"
		comboSelect += ", #{id}, #{c[:id]}Count#{i}"
		type = c[:type]
		if(type == :item)
			comboSelect += ", #{selectComboSql(type, id)} AS #{id}Name"
		elsif(type = :co)
			comboSelect += ", COALESCE(#{selectComboSql('creature', id)}"+
			", #{selectComboSql('gameobject', id)}) AS #{id}Name"
		end
	end
end

arraySelect = ''
arrays.each do |c|
	(1..c[:count]).each do |i|
		arraySelect += ", #{c[:id]}#{i}"
	end
end

# todo: prepare statements in advance.
sql = 'select quest_template.entry, zoneOrSort, minlevel, maxlevel, questlevel'+
	', requiredRaces, prevQuestId, nextQuestId, nextQuestInChain, exclusiveGroup, title, details'+
	', objectives, offerRewardText, requestItemsText, completedText'+
	', rewOrReqMoney, rewMoneyMaxLevel, rewMailTemplateId, rewMailDelaySecs'+
	comboSelect + arraySelect +
	' from quest_template' +
	comboJoin +
	' where quest_template.entry = ?'
stm = TDB::C.prepare(sql)
stm.execute(@id)
@template = stm.fetch
def translateQuestText(sym)
	bPattern = /\$[bB]/
	@template[sym].gsub!(/</, '&lt;') if(@template[sym])
	@template[sym].gsub!(/>/, '&gt;') if(@template[sym])
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
	@rewXp = (rewMoneyMaxLevel / 4.8).to_i
elsif(questLevel == 63)
	@rewXp = (rewMoneyMaxLevel / 3.6).to_i
elsif(questLevel == 62)
	@rewXp = (rewMoneyMaxLevel / 2.4).to_i
elsif(questLevel == 61)
	@rewXp = (rewMoneyMaxLevel / 1.2).to_i
else	#if(questLevel <= 60)
	@rewXp = (rewMoneyMaxLevel / 0.6).to_i
end

zoneOrSort = @template[:zoneOrSort]
if(zoneOrSort < 0)
	@sort = -zoneOrSort
	@zone = nil
else
	@sort = nil
	@zone = zoneOrSort
end

def rewRepValue(i)
	val = @template["rewRepValue#{i}"]
	if(val == 0)
		id = @template["rewRepValueId#{i}"]
		val = 0 if(id == 0)
		val = QUEST_FACTION_REWARD[1][id] if(id > 0)
		val = QUEST_FACTION_REWARD[2][-id] if(id < 0)
	end
	return val
end

def link(type, id, name)
	"<a href=\"#{type}=#{id}\">#{name}</a>"
end

def spellHtml(spellId)
	link('spell', spellId, spellId)
end

# co = creature/object
def coObjectiveHtml(i)
	coId = @template["reqCreatureOrGOId#{i}"]
	name = @template["reqCreatureOrGOId#{i}Name"]
	spellId = @template["reqSpellCast#{i}"]
	count = @template["reqCreatureOrGOCount#{i}"]
	return nil if(coId == 0 && spellId == 0)
	if(spellId == 0)
		if(coId > 0)
			html = "Kill"
		else	# (coId < 0)
			html = "Use"
		end
	else
		html = "Cast #{spellHtml(spellId)}"
		html += " on" if(coId != 0)
	end
	if(coId > 0)
		html += ' '+link('npc', coId, name)
	elsif(coId < 0)
		html += ' '+link('object', -coId, name)
	end	# (coId == 0)
	if(count != 1)
		html += " x#{count}"
	end
	html += '<br>'
	return html
end

def fetchQuest(stm, id)
	return nil if(id == 0)
	stm.execute(id)
	return stm.fetch
end

@prevQuest = fetchQuest(stm, @template[:prevQuestId])
@nextQuest = fetchQuest(stm, @template[:nextQuestId])
@nextQuestInChain = fetchQuest(stm, @template[:nextQuestInChain])
if(!@prevQuest)
	stm = TDB::C.prepare('select entry, zoneOrSort, minlevel, maxlevel, questlevel'+
		', requiredRaces, prevQuestId, nextQuestId, nextQuestInChain, title, details'+
		', objectives, offerRewardText, requestItemsText, completedText'+
		' from quest_template where (nextQuestId = ?) AND (entry != ?)')
	stm.execute(@id, @id)
	@prevQuest = stm.fetch_all
	@prevQuest = nil if(@prevQuest.empty?)
else
	@prevQuest = [@prevQuest]
end

exclusive = @template[:exclusiveGroup]
if(exclusive > 0)
	stm = TDB::C.prepare('select entry, title'+
		' from quest_template where (exclusiveGroup = ?) AND (entry != ?)')
	stm.execute(exclusive, @id)
	@exclusiveGroup = stm.fetch_all
else
	@exclusiveGroup = nil
end

stm = TDB::C.prepare('select entry, title'+
	' from quest_template where (nextQuestInChain = ?) AND (entry != ?) AND (entry != ?) AND (nextQuestId != ?)')
stm.execute(@id, @id, @template[:PrevQuestId], @id)
@optionalPrev = stm.fetch_all

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
	subSimpleTag(b, 'ol')
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
		url.gsub!(/http:\/\/.+\.wowwiki\.com\/(.+)/, 'http://www.wowpedia.org/\1')
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
