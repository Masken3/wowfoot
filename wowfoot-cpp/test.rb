#!/usr/bin/ruby
# this is a test program for wowfoot-webrick or wowfoot-cpp.

STDOUT.sync = true

VERSION_CPP = true

require '../wowfoot-webrick/src/config.rb'
require '../wowfoot-webrick/src/dbiExtras.rb'
require '../wowfoot-webrick/src/util.rb'
require 'net/http'

include Net

module TDB	# Trinity DataBase
	C = DBI::connect('dbi:Mysql:'+TDB_DATABASE, TDB_USER, TDB_PASSWORD)
	DBI::setupExtras(C)
end

# set up variables
TESTED_URLS = {}
RESULT_CODES = {}


RESULT_CODES[200] = 0
puts "Reading previous results..."
successFile = open('testedUrls.txt', 'rb+')
successFile.each do |url|
	TESTED_URLS[url.strip] = 200
	RESULT_CODES[200] += 1
end
puts "#{RESULT_CODES[200]} results read."
# successFile writes will append now.
successFile.sync = true


URLS_TO_TEST = []

def generateUrlsToTest

require '../wowfoot-ex/output/WorldMapArea.rb'
require '../wowfoot-ex/output/Achievement.rb'
require '../wowfoot-ex/output/Title.rb'
require '../wowfoot-ex/output/Faction.rb'
require '../wowfoot-ex/output/ItemSet.rb'
require '../wowfoot-ex/output/Spell.rb'

searches = [
'foo',
'bar',
'hell',
'the people',
]

dbc_pages = {
'title' => TITLE,
'achievement' => ACHIEVEMENT,
'faction' => FACTION,
'itemset' => ITEM_SET,
'spell' => SPELL,
'zone' => WORLD_MAP_AREA,
}

sql_pages = {
'npc' => { :table => 'creature_template' },
'object' => { :table => 'gameobject_template' },
'quest' => { :table => 'quest_template', :entry => 'id' },
'item' => { :table => 'item_template' },
}

base_url = "http://localhost:#{VERSION_CPP ? 3002 : WEBRICK_PORT}/"

	#URLS_TO_TEST << "#{base_url}zones"
	URLS_TO_TEST << "#{base_url}quests"
	URLS_TO_TEST << "#{base_url}items"
	URLS_TO_TEST << "#{base_url}spells"


	# populate test list
	searches.each do |s|
		url = "#{base_url}search=#{s}"
		URLS_TO_TEST << url
	end

	dbc_pages.each do |name, dbc|
		dbc.keys.sort.each do |id|
			url = "#{base_url}#{name}=#{id}"
			URLS_TO_TEST << url
		end
	end

	sql_pages.each do |name, data|
		entry = data[:entry]
		entry = 'entry' if(!entry)
		stm = TDB::C.prepare("select #{entry} from #{data[:table]}")
		stm.execute
		rows = stm.fetch_all
		rows.each do |row|
			URLS_TO_TEST << "#{base_url}#{name}=#{row[entry]}"
		end
	end
end

URL_FILENAME = 'urlsToTest.txt'

if(File.exist?(URL_FILENAME))
	puts "Reading test set..."
	file = open(URL_FILENAME, 'rb')
	file.each do |line|
		URLS_TO_TEST << line.strip
	end
	file.close
	puts "#{URLS_TO_TEST.size} urls found."
else
	generateUrlsToTest
	file = open(URL_FILENAME, 'wb')
	file.write(URLS_TO_TEST.join("\n"))
	file.close
	puts "#{URLS_TO_TEST.size} urls generated."
end


# the test functions
def testUrl(url)
	return nil if(TESTED_URLS[url])
	puts "GET #{url}"
	resp = HTTP.get_response(URI.parse(URI.escape(url)))
	code = resp.code
	TESTED_URLS[url] = code
	if(RESULT_CODES[code])
		RESULT_CODES[code] += 1
	else
		RESULT_CODES[code] = 1
	end
	parse(resp.body)
	return code
end

def parse(body)
	#todo
end


# run the tests

URLS_TO_TEST.each do |url|
	code = testUrl(url)
	next if(!code)
	break if(code.to_i != 200)
	successFile.puts(url)
end

# print results
puts "#{RESULT_CODES.size} different result codes:"
RESULT_CODES.each do |code, count|
	puts "#{code}: #{count} hits"
end
