#!/usr/bin/ruby

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/exe.rb'
require File.expand_path '../rules/dll.rb'

require './chtmlCompiler.rb'
require './ex_template.rb'
require 'erb'
require './handlers/tdb/tdb.rb'
require './config.rb'

CHTML_BUILDDIR = 'build/chtml'
TDB_BUILDDIR = 'build/tdb'
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
	@SOURCES = ['win32']
	@EXTRA_INCLUDES = ['win32']
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
			TDB_BUILDDIR,
			'win32',
			'handlers', "handlers/#{name}", '../wowfoot-ex/output',
		]
		handlerDeps.each do |dll|
			@EXTRA_INCLUDES << "handlers/#{dll}"
			f ="build/#{dll}"
			@EXTRA_INCLUDES << f if(File.exist?(f))
		end
		@PREREQUISITES = [
			DirTask.new(self, CHTML_BUILDDIR),
			DirTask.new(self, TDB_BUILDDIR),
		]
		@LOCAL_DLLS = handlerDeps
		@LOCAL_DLLS << 'common' if(hasChtml)
		@LOCAL_DLLS << 'win32' if(HOST == :win32)
		@NAME = name
		WORKS << self
	end
end

class TdbWork < HandlerWork
	def initialize(name)
		super(name, ['tdb'])
		#@EXTRA_SOURCETASKS << TdbSourceTask.new(self, name)
		@EXTRA_CPPFLAGS = ' -Wno-invalid-offsetof'
		@PREREQUISITES = [
			TdbStructHeaderTask.new(self, name),
			TdbFormatHeaderTask.new(self, name),
		]
	end
end

class PageWork < HandlerWork
	def initialize(name, handlerDeps = [])
		super
		@EXTRA_OBJECTS = [FileTask.new(self, "handlers/#{name}/#{name}.def")] if(HOST == :win32)
	end
end

class ExTemplateWork < HandlerWork
	def initialize(name, singular, plural, upperCase)
		super(name)
		@SOURCES = []
		@EXTRA_INCLUDES << "build/#{name}"
		@EXTRA_SOURCETASKS << ExTemplateCpp.new(self, name, singular, plural, upperCase)
		@EXTRA_SOURCEFILES << "../wowfoot-ex/output/#{name}.data.cpp"
	end
end

# HandlerWorks

HandlerWork.new('tdb').instance_eval do
	@EXTRA_CPPFLAGS = ' -Wno-shadow -Wno-attributes'	# mysql++ header bugs
	@LIBRARIES = ['mysqlclient']
	if(HOST == :win32)
		@EXTRA_INCLUDES += CONFIG_MYSQL_INCLUDES
		@EXTRA_LINKFLAGS = CONFIG_MYSQL_LIBDIRS
		@LIBRARIES << 'ws2_32'
	else
		@EXTRA_INCLUDES << '/usr/include/mysql'
	end
end

TdbWork.new('db_item')

ExTemplateWork.new('AreaTable', 'Area', 'AreaTable', 'AREA_TABLE')
ExTemplateWork.new('WorldMapArea', 'WorldMapArea', 'WorldMapAreas', 'WORLD_MAP_AREA')
ExTemplateWork.new('Spell', 'Spell', 'Spells', 'SPELL')
ExTemplateWork.new('TotemCategory', 'TotemCategory', 'TotemCategories', 'TOTEM_CATEGORY')

HandlerWork.new('tabs')
HandlerWork.new('tabTable', ['tabs'])
HandlerWork.new('mapSize')
HandlerWork.new('comments', ['tabs']).instance_eval do
	@LIBRARIES = ['sqlite3']
	patch = FileTask.new(self, 'build/patch.cpp')
	patch.instance_eval do
		@src = 'handlers/comments/patch.rb'
		@prerequisites = [FileTask.new(@work, @src)]
		def execute
			sh "ruby #{@src} #{@NAME}"
		end
	end
	@EXTRA_SOURCETASKS << patch
end

PageWork.new('zone', ['AreaTable', 'WorldMapArea', 'mapSize'])
PageWork.new('search', ['AreaTable', 'WorldMapArea', 'tabs', 'tabTable', 'Spell', 'db_item'])
PageWork.new('item', ['tabs', 'tabTable', 'db_item', 'TotemCategory', 'comments'])

@wfc = ExeWork.new
@wfc.instance_eval do
	@SOURCES = ['.']
	@LIBRARIES = ['microhttpd']
	@EXTRA_INCLUDES = ['win32']#'src', 'src/libs/libmpq']
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

target :callgrind => :default do
	sh "valgrind --tool=callgrind #{cmd}"
end

Targets.invoke
