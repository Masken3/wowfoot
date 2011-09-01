#!/usr/bin/ruby -w

require 'mysql'
require 'fileutils'
require './config.rb'
require '../wowfoot-ex/output/Achievement.rb'
require '../wowfoot-ex/output/Faction.rb'
require '../wowfoot-ex/output/ItemSet.rb'
require '../wowfoot-ex/output/WorldMapArea.rb'
require '../wowfoot-ex/output/Spell.rb'

@hasPrintedVersion = false

def download(name, query, num_length)
begin
	file = open('curl-config.txt', 'w')
	file.puts 'header = "accept-encoding: gzip"'
	file.puts 'max-time = 10'

	# connect to the MySQL server
	dbh = Mysql.real_connect(SQL_ADDRESS, SQL_USER, SQL_PASS, SQL_DATABASE)
	# get server version string and display it
	if(!@hasPrintedVersion)
		puts "Server version: " + dbh.get_server_info
		@hasPrintedVersion = true
	end

	res = dbh.query(query)
	nFiles = 0

	dstDir = "#{DST}#{name}"

	FileUtils.mkdir(dstDir) if(!File.exist?(dstDir))

	puts query
	while row = res.fetch_row do
		#p row
		q = row[0].to_s
		while(q.length < num_length)
			q = '0' + q
		end
		next if(q.length > num_length)	# avoid custom additions.
		fn = "#{dstDir}/#{name}=#{q}.gz"
		if(!File.exist?(fn))
			file.puts "url = \"http://old.wowhead.com/#{name}=#{q}"
			file.puts "output = \"#{fn}\""
			nFiles += 1
		end
	end
	puts "Number of rows returned: #{res.num_rows}"
	puts "Number of files to retrieve: #{nFiles}"
	res.free

	file.close

	return if(nFiles == 0)

	cmd = 'C:/path/curl -K curl-config.txt'
	puts cmd
	if(!system(cmd))
		puts "Curl error: #{$?}"
		raise hell
	end
rescue Mysql::Error => e
	puts "Error code: #{e.errno}"
	puts "Error message: #{e.error}"
	puts "Error SQLSTATE: #{e.sqlstate}" if e.respond_to?("sqlstate")
ensure
	# disconnect from server
	dbh.close if dbh
end
end

def downloadMapId(name, map, num_length)
	file = open('curl-config.txt', 'w')
	file.puts 'header = "accept-encoding: gzip"'
	file.puts 'max-time = 10'

	nFiles = 0
	dstDir = "#{DST}#{name}"
	FileUtils.mkdir(dstDir) if(!File.exist?(dstDir))

	map.each do |id, hash|
		q = id.to_s
		while(q.length < num_length)
			q = '0' + q
		end
		next if(q.length > num_length)	# avoid custom additions.
		fn = "#{dstDir}/#{name}=#{q}.gz"
		if(!File.exist?(fn))
			file.puts "url = \"http://old.wowhead.com/#{name}=#{q}"
			file.puts "output = \"#{fn}\""
			nFiles += 1
		end
	end
	puts name
	puts "Number of rows: #{map.size}"
	puts "Number of files to retrieve: #{nFiles}"

	file.close

	return if(nFiles == 0)

	cmd = 'C:/path/curl -K curl-config.txt'
	puts cmd
	if(!system(cmd))
		puts "Curl error: #{$?}"
		raise hell
	end
end

# todo: add spells
downloadMapId('spell', SPELL, 5)
downloadMapId('achievement', ACHIEVEMENT, 4)
downloadMapId('zone', WORLD_MAP_AREA, 4)
downloadMapId('faction', FACTION, 4)
downloadMapId('itemset', ITEM_SET, 4)
download('quest', 'SELECT entry FROM quest_template', 5)
download('npc', 'SELECT entry FROM creature_template', 5)
download('object', 'SELECT entry FROM gameobject_template', 6)
download('item', 'SELECT entry FROM item_template', 5)
