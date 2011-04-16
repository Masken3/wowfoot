#!/usr/bin/ruby -w

require 'mysql'
require 'config.rb'

NUM_LENGTH = 5

FILE = open('curl-config.txt', 'w')

FILE.puts 'header = "accept-encoding: gzip"'
FILE.puts 'max-time = 10'

begin
	# connect to the MySQL server
	dbh = Mysql.real_connect(SQL_ADDRESS, SQL_USER, SQL_PASS, SQL_DATABASE)
	# get server version string and display it
	puts "Server version: " + dbh.get_server_info

	res = dbh.query('SELECT entry FROM quest_template')
	nFiles = 0

	while row = res.fetch_row do
		#p row
		q = row[0].to_s
		while(q.length < NUM_LENGTH)
			q = '0' + q
		end
		if(!File.exist?("quest=#{q}.gz"))
			FILE.puts "url = \"http://old.wowhead.com/quest=#{q}"
			FILE.puts "output = \"quest=#{q}.gz\""
			nFiles += 1
		end
	end
	puts "Number of rows returned: #{res.num_rows}"
	puts "Number of files to retrieve: #{nFiles}"
	res.free
rescue Mysql::Error => e
	puts "Error code: #{e.errno}"
	puts "Error message: #{e.error}"
	puts "Error SQLSTATE: #{e.sqlstate}" if e.respond_to?("sqlstate")
ensure
	# disconnect from server
	dbh.close if dbh
end

FILE.close

puts 'Calling cURL...'
cmd = 'curl -K curl-config.txt'
puts cmd
system(cmd)
