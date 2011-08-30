

stm = CDB::C.prepare('select user, body, body as originalBody, rating, date, indent'+
	' from comments'+
	" INNER JOIN #{@commentTable}_comments on commentId = id"+
	" where #{@commentTable}_comments.entry = ?")
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
	#subSimpleTag(b, 'ul')
	b.gsub!('[ol]', '</p><ol>')
	b.gsub!('[/ol]', '</ol><p>')
	#subSimpleTag(b, 'ol')
	subSimpleTag(b, 'li')
	#subSimpleTag(b, 'quote')	# doesn't work; quote tags are complex.

	# remove extra <br> in the <ul><li> no-space.
	b.gsub!(/(<ul>[^<]*)<br>/, '\1')
	b.gsub!(/(<ol>[^<]*)<br>/, '\1')
	b.gsub!(/(<\/li>[^<]*)<br>/, '\1')

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

def commentClass(indent)
	if(indent == 0)
		 return 'rootComment'
	else
		return 'indentedComment'
	end
end

@lastIndent = -1
def indentHtmlBase(indent)
	html = ''
	i = indent
	while(i <= @lastIndent)
		html += '</div>'
		i += 1
	end
	return html
end
def indentHtml(indent)
	html = indentHtmlBase(indent)
	@lastIndent = indent
	html += "<div class=\"#{commentClass(indent)}\">"
	return html
end
def indentEndHtml
	indentHtmlBase(0)
end
