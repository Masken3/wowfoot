#!/usr/bin/ruby

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/exe.rb'
require File.expand_path '../rules/dll.rb'
require File.expand_path '../rules/native_lib.rb'
require File.expand_path '../rules/targets.rb'

LIBMPQ = DllWork.new
LIBMPQ.instance_eval do
	@SOURCES = ['src/libs/libmpq/libmpq']
	@EXTRA_INCLUDES = ['src/libs/libmpq']
	@EXTRA_CFLAGS = ' -fPIC -Wno-all -Wno-error -Wno-extra -Wno-missing-declarations'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@LIBRARIES = ['bz2', 'z']
	@NAME = 'libmpq'
end

CRBLIB = NativeLibWork.new
CRBLIB.instance_eval do
	@SOURCES = ['src/libs/blp/crblib']
	@IGNORED_FILES = ['list.c', 'myassert.c', 'floatutil.c',
		'crbeqlib.c', 'chshutil.c', 'spawnmutil.c']
	@EXTRA_INCLUDES = ['src/libs/blp']
	@EXTRA_CFLAGS = ' -fPIC -Wno-all -Wno-error -Wno-extra'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@LIBRARIES = []
	@NAME = 'crblib'
end

PALBMP = NativeLibWork.new
PALBMP.instance_eval do
	@SOURCES = ['src/libs/blp/palbmp']
	@IGNORED_FILES = []
	@EXTRA_INCLUDES = ['src/libs/blp']
	@EXTRA_CFLAGS = ' -fPIC -Wno-all -Wno-error -Wno-extra'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	@LIBRARIES = []
	@NAME = 'palbmp'
end

SQUISH = NativeLibWork.new
SQUISH.instance_eval do
	@SOURCES = ['src/libs/blp/squish']
	@EXTRA_INCLUDES = @SOURCES
	@EXTRA_CFLAGS = ' -fPIC -Wno-all -Wno-error -Wno-extra'
	@NAME = 'squish'
end

BLP = NativeLibWork.new
BLP.instance_eval do
	@SOURCES = ['src/libs/blp']
	@EXTRA_INCLUDES = ['src/libs/blp']
	@EXTRA_CFLAGS = ' -fPIC -Wno-all -Wno-error -Wno-extra'
	#@EXTRA_LINKFLAGS = ' -symbolic'
	#@LIBRARIES = ['png']	# ignored by *LibWork
	@NAME = 'libblp'
end

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
