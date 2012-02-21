#!/usr/bin/ruby

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/exe.rb'
require File.expand_path '../rules/targets.rb'
require './libs.rb'
require './config.rb'

wfe = ExeWork.new
wfe.instance_eval do
	def setup
		set_defaults

		@SOURCES = ['src', 'src/libs', 'src/libs/map']
		@EXTRA_INCLUDES = ['src', 'src/libs/libmpq']
		@IGNORED_FILES = []
		@EXTRA_CFLAGS = ' -D_POSIX_SOURCE'	#avoid silly bsd functions
		@EXTRA_OBJECTS = [LIBMPQ.target, BLP.target, SQUISH.target, PALBMP.target, CRBLIB.target]
		@SPECIFIC_CFLAGS = {
			'loadlib.cpp' => ' -Wno-multichar',
			'adt.cpp' => ' -Wno-multichar',
			'wdt.cpp' => ' -Wno-multichar',
		}
		@LIBRARIES = ['png', 'jpeg']
		@EXTRA_LINKFLAGS = CONFIG_LOCAL_LIB

		@NAME = 'wowfoot-ex'

		super
	end
end

target :default do
	LIBMPQ.invoke
	CRBLIB.invoke
	PALBMP.invoke
	SQUISH.invoke
	BLP.invoke
	wfe.invoke
end

target :run => :default do
	sh "#{wfe.target}"
end

target :gdb => :default do
	sh "gdb --args #{wfe.target}"
end

target :mc => :default do
	sh "valgrind --leak-check=full #{wfe.target}"
end

Targets.invoke
