#!/usr/bin/ruby

# todo: connect to the Trinity DB to verify that we have all quests.

# for now: read quest files from 20k disk files and store the comments
# in an sqlite DB for easy access.

require 'rubygems'	# needed for sqlite3 on some(unknown) versions of Ruby
require 'fileutils'
require 'sqlite3'
require './config.rb'
require 'json/pure'
require './json.rb'

# static config
DB_FILENAME = 'imports.db'

# helpers
class String
	def beginsWith(other)
		os = other.to_s
		return self[0,os.length] == (os)
	end
end

# main program
db = SQLite3::Database.new(DB_FILENAME)
File.open('create-db.sql', 'r') do |infile|
	while (line = infile.gets)
		if(line.length != 1 && !line.empty?)
			db.execute line
		end
	end
end

COMMENT_LINE_BEGIN = 'var lv_comments0 = '
def parseCommentLine(entry, line)
	pos = COMMENT_LINE_BEGIN.length
	open('file.txt', 'w').write("#{entry}, #{line}")
	if(false)	# json standard requires that key strings be quoted, so we can't use this parser.
		line = line[pos, line.length-(pos+2)]
		line.gsub!("'", '"')
		#p line
		hashArray = JSON.parse(line)
	else
		#p line[pos, line.length]
		hashArray = JsonParser.parse(line, pos)
	end
	p hashArray
	query = ''
	hashArray.each do |h|
		query += "INSERT INTO 'quest_comments' VALUES (#{entry}, #{h[:id]});\n"
		query += "INSERT INTO 'comments' VALUES (#{h[:id]}, '#{h[:user]}', '#{h[:body]}',"+
			" #{h[:rating]}, '#{h[:date]}', #{h[:indent]}});\n"
	end
	return query
end

query = "BEGIN;\n"
Dir[CONFIG_SRCDIR + '/*'].each do |filename|
	p filename
	entry = File.basename(filename)['quest='.length, filename.length]
	open(filename, 'r') do |qfile|
		while(line = qfile.gets)
			if(line.beginsWith(COMMENT_LINE_BEGIN))
				query += parseCommentLine(entry, line)
			end
		end
	end
end
query += "COMMIT;\n"

open('query.sql', 'w').write(query)
db.execute query
