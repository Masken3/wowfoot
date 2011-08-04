
# todo: prepare statements in advance.
stm = TDB::C.prepare('select entry, zoneOrSort, minlevel, maxlevel, questlevel'+
	', requiredRaces, prevQuestId, nextQuestId, nextQuestInChain, title, details'+
	', objectives, offerRewardText, requestItemsText, completedText'+
	' from quest_template where entry = ?')
stm.execute(@id)
@template = stm.fetch
bPattern = /\$[bB]/
@template[:details].gsub!(bPattern, '<br>') if(@template[:details])
@template[:offerRewardText].gsub!(bPattern, '<br>') if(@template[:offerRewardText])
@template[:requestItemsText].gsub!(bPattern, '<br>') if(@template[:requestItemsText])
@template[:completedText].gsub!(bPattern, '<br>') if(@template[:completedText])

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
@nextQuest = fetchQuest(stm, @template[:nextQuestId])
@nextQuestInChain = fetchQuest(stm, @template[:nextQuestInChain])
if(!@prevQuest)
	stm = TDB::C.prepare('select entry, zoneOrSort, minlevel, maxlevel, questlevel'+
		', requiredRaces, prevQuestId, nextQuestId, nextQuestInChain, title, details'+
		', objectives, offerRewardText, requestItemsText, completedText'+
		' from quest_template where (nextQuestId = ? OR nextQuestInChain = ?) AND (entry != ?)')
	stm.execute(@id, @id, @id)
	@optionalPrev = stm.fetch
else
	@optionalPrev = nil
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

stm = CDB::C.prepare('select user, body, rating, date, indent'+
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
	p b
	b.gsub!('\\n', "<br>\n")
	subSimpleTag(b, 'b')
	subSimpleTag(b, 'i')
	subSimpleTag(b, 'ul')
	subSimpleTag(b, 'li')
	
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
