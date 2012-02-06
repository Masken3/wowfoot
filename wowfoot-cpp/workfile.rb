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
WORK_MAP = {}

# setup LIBMPQ. invoke later.
include FileUtils::Verbose
HOME_DIR = pwd
FileUtils.cd '../wowfoot-ex'
require './libs.rb'
LIBMPQ.setup
BLP.setup
SQUISH.setup
PALBMP.setup
CRBLIB.setup
FileUtils.cd HOME_DIR

if(true)#HOST == :win32)
def rootSendSignal(works)
	puts "Signalling (#{works})..."
	res = Net::HTTP.get_response('localhost', '/unload=' + works, 3002)
	p res
	p res.body
end

def findPrerequisites(main, level=0)
	a = ''
	WORKS.each do |w| w.prerequisites.each do |pre|
		next unless(pre.to_s.endsWith(DLL_FILE_ENDING))
		name = File.basename(pre.to_s, DLL_FILE_ENDING)
		pre.prerequisites.each do |p|
			next unless(p.to_s.endsWith(DLL_FILE_ENDING))
			pn = File.basename(p.to_s, DLL_FILE_ENDING)
			if(pn == main)
				a << name+','
				puts 'Found: ' + main + ' in ' + (' '*level) + name
				a << findPrerequisites(name, level + 1)
			end
		end
	end; end
	return a
end

# force server to unload DLLs,
# to remove the write protection.
class DllTask
	alias_method :old_execute, :execute

	def sendSignal
		if(HOST != :win32)
			#setNewName	# causes crash
			return
		end
		# send a signal containing all the
		# idHandlers that depends on this DLL (including itself).
		# wait for a "finished" signal from the server.
		idHandlerWorks = ''
		main = File.basename(self.to_s, DLL_FILE_ENDING)

		idHandlerWorks << findPrerequisites(main)
		#if(@work.respond_to?(:newDllName) && HOST == :linux)
		if(false)
			setNewName
			idHandlerWorks << main << ':' << @NAME
		end
		rootSendSignal(idHandlerWorks) if(!idHandlerWorks.empty?)
	end
	def readCount
		cn = "#{@work.baseName}.count"
		c = 0
		if(File.exist?(cn))
			open(cn) do |file|
				c = file.read.strip.to_i
				file.close
			end
		end
		#puts "#{@NAME}: loaded #{isLoaded}, needed #{p_needed?}, count #{c}" if(@NAME.include?('icon'))
		#@work.dump(0) if(@NAME.include?('icon'))
		return c
	end

	def writeCount(c)
		cn = "#{@work.baseName}.count"
		c += 1
		open(cn, 'w') do |file|
			file.write(c.to_s)
			file.close
		end
		return c
	end
	def newDllName(c)
		return File.expand_path "#{@work.baseName}.#{c}#{DLL_FILE_ENDING}"
	end
	def setNewName
		return if(!isLoaded)
		c = readCount
		if(c != 0)
			@originalName = @NAME
			@NAME = newDllName(c)
		end
		c = writeCount(c) if(p_needed?)
		if(c != 0)
			@originalName = @NAME if(!@originalName)
			@NAME = newDllName(c)
			puts "#{@originalName} is loaded. renaming to #{@NAME}"
		end
	end
	def isLoaded; if(HOST == :win32)
		# try to remove target file.
		# if that fails and the file still exists, it's loaded.
		FileUtils.rm_f(@NAME)
		return File.exist?(@NAME)
	else	# unix-type systems
		if(@work.is_a?(PageWork))
			#puts "#{@NAME} is a PageWork."
			return false
		end
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
						#puts "Found #{@NAME} in #{proc}"
						return true
					end
				end
			end;end
		end
		return false
	end;end
	def execute
		sendSignal if(isLoaded)
		FileUtils.rm_f(@NAME)
		raise "Loaded file was not deleted!" if(File.exist?(@NAME))
		old_execute
		#cp(@NAME, @originalName) if(@originalName)
	end
end
end	# WIN32

common = NativeLibWork.new
common.instance_eval do
	@SOURCES = ['handlers', 'util']
	if(HOST == :win32)
		@SOURCES << 'util/win32'
		@SOURCES << 'util/win32/sym_engine'
		#@LIBRARIES = ['imagehlp']
	else
		@SOURCES << 'util/unix'
		#@LIBRARIES = ['dl']
	end
	@SPECIFIC_CFLAGS = {
		'process.cpp' => ' -Wno-missing-format-attribute',
	}
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
			'handlers', "handlers/#{name}", '../wowfoot-ex/output',
			'.', CHTML_BUILDDIR,
			TDB_BUILDDIR,
			'win32',
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
		@LOCAL_LIBS = ['common']
		@LIBRARIES = []
		@LIBRARIES << 'imagehlp' if(HOST == :win32)
		@LIBRARIES << 'dl' if(HOST != :win32)
		@LOCAL_DLLS << 'win32' if(HOST == :win32)
		@NAME = name
		WORKS << self
		WORK_MAP[name] = self
	end
	def name; File.basename(@TARGET.to_s, '.so'); end
	def baseName; "#{@BUILDDIR}#{@NAME}"; end
	if(HOST != :win32) then def setup
		@LOCAL_DLLS = @LOCAL_DLLS.collect do |dll|
			w = WORK_MAP[dll]
			if(w)
				dll = w.name
			end
			dll
		end
		super
		@TARGET.setNewName
	end end
end

class TdbWork < HandlerWork
	def initialize(name, handlerDeps = [])
		super(name, ['tdb'] + handlerDeps)
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
HandlerWork.new('areaMap')

HandlerWork.new('tdb').instance_eval do
	@EXTRA_CPPFLAGS = ' -Wno-shadow -Wno-attributes'	# mysql++ header bugs
	@LIBRARIES << 'mysqlclient'
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

HandlerWork.new('dbcItemSubClass', ['dbc']).instance_eval do
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs'
end
HandlerWork.new('dbcItemClass', ['dbc']).instance_eval do
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs'
end
HandlerWork.new('icon', ['dbc']).instance_eval do
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs'
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs/libmpq'
	@prerequisites << DirTask.new(self, 'build/icon')
	set_defaults
	def copy(work)
		CopyFileTask.new(self, @BUILDDIR + File.basename(work.target.to_s),
			FileTask.new(self, work.target.to_s))
	end
	@EXTRA_OBJECTS = [copy(LIBMPQ), copy(BLP), copy(SQUISH), copy(PALBMP), copy(CRBLIB)]
	@LIBRARIES += ['png', 'jpeg']
end

TdbWork.new('db_achievement_reward')
TdbWork.new('db_quest')
TdbWork.new('db_loot_template')
TdbWork.new('db_item')
TdbWork.new('db_npc_vendor')
TdbWork.new('db_spawn')
TdbWork.new('db_creature_template', ['db_spawn'])
TdbWork.new('db_gameobject_template', ['db_spawn'])

DbcWork.new('dbcItemDisplayInfo')
DbcWork.new('dbcItemSet')
DbcWork.new('dbcCharTitles')
DbcWork.new('dbcQuestFactionReward')
DbcWork.new('dbcFaction')
DbcWork.new('dbcAchievement')
DbcWork.new('dbcArea')
DbcWork.new('dbcSpell')
DbcWork.new('dbcWorldMapArea')
DbcWork.new('dbcTotemCategory')
DbcWork.new('dbcItemExtendedCost', ['db_npc_vendor', 'db_creature_template', 'db_item'])
DbcWork.new('dbcChrClasses')
DbcWork.new('dbcChrRaces')

HandlerWork.new('tabs')
HandlerWork.new('tabTable', ['tabs'])
HandlerWork.new('mapSize')
HandlerWork.new('comments', ['tabs', 'dbcSpell', 'db_item', 'dbcWorldMapArea',
	'dbcAchievement', 'db_quest', 'db_creature_template',
	'db_gameobject_template', 'dbcFaction',
]).instance_eval do
	@LIBRARIES << 'sqlite3'
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
HandlerWork.new('spawnPoints', ['mapSize', 'dbcArea', 'dbcWorldMapArea', 'areaMap'])

PageWork.new('quests', ['tabTable', 'tabs', 'db_quest', 'dbcFaction'])
PageWork.new('title', ['dbcAchievement', 'tabTable', 'tabs', 'comments', 'dbcCharTitles',
	'db_achievement_reward'])
PageWork.new('faction', ['tabTable', 'tabs', 'comments', 'dbcFaction'])
PageWork.new('quest', ['tabTable', 'tabs', 'comments', 'db_quest', 'dbcSpell', 'db_creature_template',
	'db_item', 'dbcFaction', 'dbcQuestFactionReward'])
PageWork.new('achievement', ['dbcAchievement', 'tabs', 'comments', 'dbcCharTitles',
	'db_item', 'db_creature_template', 'db_achievement_reward'])
PageWork.new('npc', ['db_creature_template',
	'db_spawn', 'tabs', 'comments', 'spawnPoints', 'mapSize'])
PageWork.new('zone', ['dbcArea', 'dbcWorldMapArea', 'mapSize'])
PageWork.new('search', ['dbcArea', 'dbcWorldMapArea', 'tabs', 'tabTable', 'dbcSpell', 'db_item',
	'db_creature_template', 'dbcAchievement', 'db_gameobject_template', 'db_quest',
	'dbcCharTitles', 'dbcItemSet'])
PageWork.new('item', ['tabs', 'tabTable', 'db_item', 'dbcTotemCategory', 'comments',
	'db_npc_vendor', 'db_creature_template', 'dbcItemExtendedCost', 'dbcSpell',
	'db_loot_template', 'dbcChrClasses', 'dbcChrRaces', 'db_gameobject_template',
	'dbcItemClass', 'dbcItemSubClass', 'dbcItemSet', 'icon', 'dbcItemDisplayInfo'])
PageWork.new('itemset', ['tabs', 'tabTable', 'db_item', 'dbcTotemCategory', 'comments',
	'db_npc_vendor', 'db_creature_template', 'dbcItemExtendedCost', 'dbcSpell',
	'db_loot_template', 'dbcChrClasses', 'dbcChrRaces', 'db_gameobject_template',
	'dbcItemClass', 'dbcItemSubClass', 'dbcItemSet', 'item'])
PageWork.new('object', ['db_gameobject_template',
	'db_spawn', 'tabs', 'comments', 'spawnPoints', 'mapSize', 'item', 'tabTable',
	'db_loot_template', 'db_item'])
PageWork.new('spell', ['tabs', 'tabTable', 'db_item', 'comments', 'dbcSpell',
	'db_creature_template'])
PageWork.new('items', ['db_item', 'dbcItemClass', 'dbcItemSubClass', 'item', 'tabTable', 'tabs'])

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
	rm_f 'build/count'
	rm_f Dir.glob('build/*/*.count')
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
