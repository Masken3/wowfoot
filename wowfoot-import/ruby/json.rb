# helpers
class String
	def beginsWith(other)
		os = other.to_s
		return self[0,os.length] == (os)
	end
end

def matchchar(c, m)
	s = ' '
	s[0] = c
	return s.match(m) != nil
end

def isdigit(c)
	return c == '-'[0] || matchchar(c, /\d/)
end

def isspace(c)
	matchchar(c, /\s/)
end

def isletter(c)
	matchchar(c, /[A-Za-z]/)
end

class JsonParser
	def self.parse(line, pos)
		@@line = line
		@@pos = pos
		begin
			return parseValue
		rescue
			p "line: " + @@line
			puts "length: #{@@line.length}"
			puts "pos #{@@pos}"
			puts "place: #{@@line[@@pos, @@line.length]}"
			raise
		end
	end
	
	def self.parseValue
		#puts 'pV'
		skipWhitespace
		c = @@line[@@pos,1]
		return parseArray if(c == '[')
		return parseObject if(c == '{')
		return parseString(c) if(c == '"' || c == "'")
		return parseInt if(isdigit(c[0]))
		error
	end
	
	def self.parseString(delimiter)
		token(delimiter)
		s = ''
		while(@@line[@@pos,1] != delimiter)
			if(@@line[@@pos,1] == "\\")
				@@pos += 1
			end
			s += @@line[@@pos,1]
			@@pos += 1
		end
		@@pos += 1
		return s
	end
	
	def self.parseInt
		pos = @@pos
		while(isdigit(@@line[@@pos]))
			@@pos += 1
		end
		len = @@pos - pos
		return @@line[pos, len].to_i
	end
	
	# returns an array of values (integers, strings, hashes, arrays).
	def self.parseArray
		token('[')
		#puts 'pA'
		arr = []
		while(true)
			arr << parseValue
			
			if(next?(']'))
				next
			else
				break
			end
		end
		return arr
	end
	
	# returns a hash (string => value). that's what a JSON object is.
	def self.parseObject
		token('{')
		hash = {}
		while(true)
			skipWhitespace
			namePos = @@pos
			while(isletter(@@line[@@pos]))
				@@pos += 1
			end
			nameLen = @@pos - namePos
			name = @@line[namePos, nameLen]
			token(':')
			hash[name.to_sym] = parseValue
			
			if(next?('}'))
				next
			else
				break
			end
		end
		return hash
	end

	# helpers
	def self.error
		p @@line[@@pos-1, @@line.length]
		raise hell
	end
	
	def self.next?(breakToken)
		c = @@line[@@pos,1]
		@@pos += 1
		return true if(c == ',')
		return false if(c == breakToken)
		error
	end
		
	def self.skipWhitespace
		while(isspace(@@line[@@pos]))
			@@pos += 1
		end
	end
	def self.token(t)
		skipWhitespace
		error unless(@@line[@@pos, t.length] == t)
		@@pos += t.length
	end
end
