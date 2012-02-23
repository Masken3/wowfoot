#!/usr/bin/ruby
# this is a test program for wowfoot-webrick or wowfoot-cpp.

STDOUT.sync = true

VERSION_CPP = true

require '../wowfoot-webrick/src/config.rb'
require '../wowfoot-webrick/src/dbiExtras.rb'
require '../wowfoot-webrick/src/util.rb'
require '../wowfoot-ex/output/WorldMapArea.rb'
require 'net/http'

include Net

module TDB	# Trinity DataBase
	C = DBI::connect('dbi:Mysql:'+TDB_DATABASE, TDB_USER, TDB_PASSWORD)
	DBI::setupExtras(C)
end

# set up variables
TESTED_URLS = {}
RESULT_CODES = {}

SEARCHES = [
'foo',
'bar',
'hell',
'the people',
]

DBC_PAGES = {
'title' => 'CharTitles',
'achievement' => 'dbcAchievement',
'zone' => 'dbcWorldMapArea',
'faction' => 'Faction',
'itemset' => 'ItemSet',
'spell' => 'Spell',
}

SQL_PAGES = {
'npc' => { :table => 'creature_template' },
'object' => { :table => 'gameobject_template' },
'quest' => { :table => 'quest_template', :entry => 'id' },
'item' => { :table => 'item_template' },
}


BASE_URL = "http://localhost:#{VERSION_CPP ? 3002 : WEBRICK_PORT}/"


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
	#URLS_TO_TEST << "#{BASE_URL}zones"
	URLS_TO_TEST << "#{BASE_URL}quests"
	URLS_TO_TEST << "#{BASE_URL}items"
	URLS_TO_TEST << "#{BASE_URL}spells"


	# populate test list
	SEARCHES.each do |s|
		url = "#{BASE_URL}search=#{s}"
		URLS_TO_TEST << url
	end

	WORLD_MAP_AREA.each do |id, data|
		URLS_TO_TEST << "#{BASE_URL}zone=#{id}"
	end

	SQL_PAGES.each do |name, data|
		entry = data[:entry]
		entry = 'entry' if(!entry)
		stm = TDB::C.prepare("select #{entry} from #{data[:table]}")
		stm.execute
		rows = stm.fetch_all
		rows.each do |row|
			URLS_TO_TEST << "#{BASE_URL}#{name}=#{row[entry]}"
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
