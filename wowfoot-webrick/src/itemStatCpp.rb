require './itemStat.rb'

file = open('itemStat.h', 'w')
ITEM_STAT.each do |id, text|
	file.puts "\tcase #{id}: \"#{text}\";"
end
