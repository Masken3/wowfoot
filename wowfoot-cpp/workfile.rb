#!/usr/bin/ruby

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/exe.rb'
require File.expand_path '../rules/dll.rb'

require './chtmlCompiler.rb'

CHTML_BUILDDIR = 'build/chtml'
WORKS = []

common = DllWork.new
common.instance_eval do
	@SOURCES = ['handlers']
	@EXTRA_INCLUDES = ['.']
	@NAME = 'common'
	WORKS << self
end

win32 = DllWork.new
win32.instance_eval do
	@SOURCES = []
	@EXTRA_INCLUDES = ['.']
	@EXTRA_SOURCEFILES = ['win32.cpp']
	@SPECIFIC_CFLAGS = {
		'win32.cpp' => ' -Wno-missing-format-attribute',
	}
	@NAME = 'win32'
end

class HandlerWork < DllWork
	def initialize(name, handlerDeps = [])
		super()
		hasChtml = false
		@EXTRA_SOURCETASKS = Dir["handlers/#{name}/*.chtml"].collect do |chtml|
			hasChtml = true
			bn = File.basename(chtml, '.chtml')
			ChtmlCompileTask.new(self, CHTML_BUILDDIR, bn, chtml)
		end
		@SOURCES = ["handlers/#{name}"]
		@EXTRA_SOURCEFILES = []
		@EXTRA_INCLUDES = [
			'.', CHTML_BUILDDIR,
			'handlers', "handlers/#{name}", '../wowfoot-ex/output',
		] + handlerDeps.collect do |dll|
			"handlers/#{dll}"
		end
		@LOCAL_DLLS = handlerDeps
		@LOCAL_DLLS << 'common' if(hasChtml)
		@LOCAL_DLLS << 'win32' if(HOST == :win32)
		@NAME = name
		WORKS << self
	end
end

HandlerWork.new('tabTables')
HandlerWork.new('mapSize')
HandlerWork.new('AreaTable').instance_eval do
	@EXTRA_SOURCEFILES << '../wowfoot-ex/output/AreaTable.data.cpp'
end
HandlerWork.new('WorldMapArea').instance_eval do
	@EXTRA_SOURCEFILES << '../wowfoot-ex/output/WorldMapArea.data.cpp'
end
HandlerWork.new('Spell').instance_eval do
	@EXTRA_SOURCEFILES << '../wowfoot-ex/output/Spell.data.cpp'
end
HandlerWork.new('zone', ['AreaTable', 'WorldMapArea', 'mapSize']).instance_eval do
	@EXTRA_OBJECTS = [FileTask.new(self, 'handlers/zone/zone.def')]
end
HandlerWork.new('search', ['AreaTable', 'WorldMapArea', 'tabTables', 'Spell']).instance_eval do
	@EXTRA_OBJECTS = [FileTask.new(self, 'handlers/zone/zone.def')]
end

@wfc = ExeWork.new
@wfc.instance_eval do
	@SOURCES = ['.']
	@LIBRARIES = ['microhttpd']
	#@EXTRA_INCLUDES = ['src', 'src/libs/libmpq']
	@IGNORED_FILES = ['win32.cpp']
	#@EXTRA_CFLAGS = ' -D_POSIX_SOURCE'	#avoid silly bsd functions

	if(HOST == :win32)
		@EXTRA_SOURCEFILES = ["dll/dll-win32.cpp"]
		@LIBRARIES << 'wsock32'
		@LOCAL_DLLS = ['win32']
	elsif(HOST == :linux || HOST == :darwin)
		@EXTRA_SOURCEFILES = ["dll/dll-unix.cpp"]
		@LIBRARIES << 'dl'
	else
		error "Unsupported platform"
	end

	@NAME = 'wowfoot-cpp'

	def buildDir; @BUILDDIR; end
end

def cmd; "#{@wfc.target} #{@wfc.buildDir}"; end

target :default do
	win32.invoke if(HOST == :win32)
	@wfc.invoke
	WORKS.each do |w|
		w.invoke
	end
end

target :run => :default do
	sh cmd
end

target :gdb => :default do
	sh "gdb --args #{cmd}"
end

target :mc => :default do
	sh "valgrind --leak-check=full #{cmd}"
end

Targets.invoke
