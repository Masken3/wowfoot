#!/usr/bin/ruby

require File.expand_path 'rules/host.rb'
require File.expand_path 'rules/exe.rb'
require File.expand_path 'rules/dll.rb'
require File.expand_path 'rules/targets.rb'

LIBMPQ = DllWork.new
LIBMPQ.instance_eval do
	@SOURCES = ['src/libs/libmpq/libmpq']
	@EXTRA_CFLAGS = ' -fPIC -Wno-all -Wno-error -Wno-extra'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@LIBRARIES = ['bz2', 'z']
	@NAME = 'libmpq'
end

wfe = ExeWork.new
wfe.instance_eval do
	def setup
		set_defaults

		@SOURCES = ['src', 'src/libs']
		@EXTRA_INCLUDES = ['src', 'src/libs/libmpq']
		@IGNORED_FILES = []
		@EXTRA_CFLAGS = ' -D_POSIX_SOURCE'	#avoid silly bsd functions
		@EXTRA_OBJECTS = [LIBMPQ.target]
		@SPECIFIC_CFLAGS = {
			'loadlib.cpp' => ' -Wno-multichar',
		}
		
		@NAME = 'wowfoot-ex'
		
		super
	end
end

target :default do
	LIBMPQ.invoke
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
