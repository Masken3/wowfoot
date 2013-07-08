#!/usr/bin/ruby
# this is a test program for wowfoot-webrick.

STDOUT.sync = true

require './src/config.rb'
require './src/dbiExtras.rb'
require './src/util.rb'
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

SQL_PAGES = {
'npc' => { :table => 'creature_template' },
'object' => { :table => 'gameobject_template' },
'quest' => { :table => 'quest_template' },
'item' => { :table => 'item_template' },
}

BASE_URL = "http://localhost:#{WEBRICK_PORT}/"

URLS_TO_TEST = {
"#{BASE_URL}areas" => true
}


puts "Reading previous results..."
if(File.exist?('testedUrls.txt'))
	successFile = open('testedUrls.txt', 'rb+')
	RESULT_CODES[200] = 0
	successFile.each do |url|
		TESTED_URLS[url] = 200
		RESULT_CODES[200] += 1
	end
	puts "#{RESULT_CODES[200]} results read."
	# successFile writes will append now.
	successFile.sync = true
else
	successFile = open('testedUrls.txt', 'wb')
end

# populate test list
SEARCHES.each do |s|
	url = "#{BASE_URL}search=#{s}"
	URLS_TO_TEST[url] = true
end

SQL_PAGES.each do |name, data|
	stm = TDB::C.prepare("select entry from #{data[:table]}")
	stm.execute
	rows =stm.fetch_all
	rows.each do |row|
		URLS_TO_TEST["#{BASE_URL}#{name}=#{row[:entry]}"] = true
	end
end

WORLD_MAP_AREA.each do |id, data|
	URLS_TO_TEST["#{BASE_URL}area=#{id}"] = true
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

URLS_TO_TEST.each do |url, dummy|
	code = testUrl(url)
	break if(code.to_i != 200)
	successFile.puts(url)
end

# print results
puts "#{RESULT_CODES.size} different result codes:"
RESULT_CODES.each do |code, count|
	puts "#{code}: #{count} hits"
end
