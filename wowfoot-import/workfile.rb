#!/usr/bin/ruby

require File.expand_path 'rules/exe.rb'
require File.expand_path 'rules/targets.rb'

work = ExeWork.new
work.instance_eval do
	@SOURCES = ['src']
	@EXTRA_INCLUDES = ['src']
	@LIBRARIES = ['sqlite3']
	@NAME = 'import'
end

target :default do
	work.invoke
end

target :run => :default do
	sh "#{work.target}"
end

target :gdb => :default do
	sh "gdb --args #{work.target}"
end

target :mc => :default do
	sh "valgrind --leak-check=full #{work.target}"
end

Targets.invoke
