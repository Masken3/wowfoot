#!/usr/bin/ruby

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/cExe.rb'
require './libs.rb'
require './config.rb'

wfe = ExeWork.new do
	@SOURCES = ['src', 'src/libs', 'src/libs/map']
	@EXTRA_INCLUDES = ['src', 'src/libs/libmpq']
	@IGNORED_FILES = []
	@EXTRA_CFLAGS = ' -D_POSIX_SOURCE'	#avoid silly bsd functions
	@EXTRA_OBJECTS = [LIBMPQ, BLP, SQUISH, PALBMP, CRBLIB]
	@SPECIFIC_CFLAGS = {
		'loadlib.cpp' => ' -Wno-multichar',
		'adt.cpp' => ' -Wno-multichar',
		'wdt.cpp' => ' -Wno-multichar',
	}
	@LIBRARIES = ['png', 'jpeg']
	@EXTRA_LINKFLAGS = CONFIG_LOCAL_LIB

	@NAME = 'wowfoot-ex'
end

target :default do
end

target :run => :default do
	sh "#{wfe}"
end

target :gdb => :default do
	sh "gdb --args #{wfe}"
end

target :mc => :default do
	sh "valgrind --leak-check=full #{wfe}"
end

Works.run
