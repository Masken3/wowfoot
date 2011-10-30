#!/usr/bin/ruby

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/exe.rb'
require File.expand_path '../rules/dll.rb'

require './chtmlCompiler.rb'
require './ex_template.rb'
require 'erb'
require './handlers/tdb/tdb.rb'
require './handlers/dbc/dbc.rb'
require './config.rb'
require 'net/http'

CHTML_BUILDDIR = 'build/chtml'
TDB_BUILDDIR = 'build/tdb'
WORKS = []

# setup LIBMPQ. invoke later.
include FileUtils::Verbose
HOME_DIR = pwd
FileUtils.cd '../wowfoot-ex'
require './libs.rb'
LIBMPQ.setup
FileUtils.cd HOME_DIR

if(true)#HOST == :win32)
def rootSendSignal(works)
	puts "Signalling (#{works})..."
	res = Net::HTTP.get_response('localhost', '/unload=' + works, 3002)
	p res
	p res.body
end

# force server to unload DLLs,
# to remove the write protection.
class DllTask
	alias_method :old_execute, :execute
	def sendSignal
		# send a signal containing all the
		# idHandlers that depends on this DLL (including itself).
		# wait for a "finished" signal from the server.
		idHandlerWorks = ''
		main = File.basename(self.to_s, '.dll')
		# wrong: this is this DLL's dependencies, not the other way around.
		WORKS.each do |w| w.prerequisites.each do |pre|
			name = pre.to_s;
			next unless(name.endsWith('.dll'))
			# this is the work's primary output. now we must look at its dependencies.
			pre.prerequisites.each do |pp|
				pn = File.basename(pp.to_s, '.dll')
				idHandlerWorks << File.basename(name, '.dll')+',' if(pn == main)
			end
		end; end
		idHandlerWorks << main
		rootSendSignal(idHandlerWorks)
	end
	def isLoaded; if(HOST == :win32)
		# try to remove target file.
		# if that fails and the file still exists, it's loaded.
		FileUtils.rm_f(@NAME)
		return File.exist?(@NAME)
	else	# unix-type systems
		# read /proc to find our server process.
		# read /proc/*/maps to list the loaded objects.
		#p WFC.target.to_s
		Dir['/proc/*'].each do |proc|
			#p proc
			# Can be optimized by caching the exe search result.
			# Seems fast enough for now.
			if(File.exists?(proc+'/exe')); if(File.readlink(proc+'/exe') == WFC.target.to_s)
				open(proc+'/maps', 'r').each do |line|
					if(line.index(@NAME))
						puts "Found #{@NAME} in #{proc}"
						return true
					end
				end
			end;end
		end
		return false
	end;end
	def execute
		sendSignal if(isLoaded)
		old_execute
	end
end
end	# WIN32

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
	def initialize(name, handlerDeps = [], isPage = false)
		super()
		hasChtml = false
		@EXTRA_SOURCETASKS = Dir["handlers/#{name}/*.chtml"].collect do |chtml|
			hasChtml = true
			bn = File.basename(chtml, '.chtml')
			ChtmlCompileTask.new(self, CHTML_BUILDDIR, bn, chtml, isPage)
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
		@EXTRA_SOURCETASKS << TdbCppTask.new(self, name)
		@EXTRA_CPPFLAGS = ' -Wno-invalid-offsetof'
		@PREREQUISITES = [
			TdbStructHeaderTask.new(self, name),
			TdbFormatHeaderTask.new(self, name),
			TdbExtHeaderTask.new(self, name),
		]
	end
end

class PageWork < HandlerWork
	def initialize(name, handlerDeps = [])
		super(name, handlerDeps, true)
		@EXTRA_OBJECTS = [FileTask.new(self, "handlers/#{name}/#{name}.def")] if(HOST == :win32)
	end
end

class ExTemplateWork < HandlerWork
	def initialize(name, singular, plural, upperCase, handlerDeps = [])
		super(name, handlerDeps)
		@EXTRA_INCLUDES << "build/#{name}"
		@EXTRA_SOURCETASKS << ExTemplateCpp.new(self, name, singular, plural, upperCase)
		@EXTRA_SOURCEFILES << "../wowfoot-ex/output/#{name}.data.cpp"
	end
end

class DbcWork < HandlerWork
	def initialize(name, handlerDeps = [])
		handlerDeps << 'dbc'
		super
		@EXTRA_INCLUDES << '../wowfoot-ex/src/libs'
		@EXTRA_INCLUDES << "build/#{name}"
		@EXTRA_SOURCETASKS << DbcCppTask.new(self, name)
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

HandlerWork.new('dbc').instance_eval do
	@SOURCES << '../wowfoot-ex/src/libs'
	@EXTRA_INCLUDES << '../wowfoot-ex/src'
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs/libmpq'
	@SPECIFIC_CFLAGS = {
		'loadlib.cpp' => ' -Wno-multichar',
	}
	set_defaults
	@EXTRA_OBJECTS = [CopyFileTask.new(self, @BUILDDIR + File.basename(LIBMPQ.target.to_s),
		FileTask.new(self, LIBMPQ.target.to_s))]
end

TdbWork.new('db_item')
TdbWork.new('db_npc_vendor')
TdbWork.new('db_creature_template')
TdbWork.new('db_creature')

DbcWork.new('dbcAchievement')
DbcWork.new('dbcArea')
DbcWork.new('dbcSpell')
DbcWork.new('dbcWorldMapArea')
DbcWork.new('dbcTotemCategory')
DbcWork.new('dbcItemExtendedCost', ['db_npc_vendor', 'db_creature_template', 'db_item'])

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

PageWork.new('achievement', ['dbcAchievement', 'tabs', 'comments'])
PageWork.new('npc', ['dbcArea', 'dbcWorldMapArea', 'mapSize', 'db_creature_template',
	'db_creature', 'tabs', 'comments'])
PageWork.new('zone', ['dbcArea', 'dbcWorldMapArea', 'mapSize'])
PageWork.new('search', ['dbcArea', 'dbcWorldMapArea', 'tabs', 'tabTable', 'dbcSpell', 'db_item',
	'db_creature_template', 'dbcAchievement'])
PageWork.new('item', ['tabs', 'tabTable', 'db_item', 'dbcTotemCategory', 'comments',
	'db_npc_vendor', 'db_creature_template', 'dbcItemExtendedCost', 'dbcSpell'])
PageWork.new('spell', ['tabs', 'tabTable', 'db_item', 'comments', 'dbcSpell',
	'db_creature_template'])

WFC = @wfc = ExeWork.new
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
	# required for unload to function properly
	WORKS.each do |w|
		w.setup
	end
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

# compile wowfoot-ex libs
include FileUtils::Verbose
FileUtils.cd '../wowfoot-ex'
LIBMPQ.invoke
FileUtils.cd HOME_DIR

Targets.invoke
