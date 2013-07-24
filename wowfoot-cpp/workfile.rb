#!/usr/bin/ruby

CONFIG_CCOMPILE_DEFAULT = 'debug'

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/cExe.rb'
require File.expand_path '../rules/cDll.rb'
require File.expand_path '../rules/cLib.rb'

require './config.rb'
require './chtmlCompiler.rb'
require './ex_template.rb'
require 'erb'
require './handlers/tdb/tdb.rb'
require './handlers/dbc/dbc.rb'
require 'net/http'
require 'stringio'

#sh 'rm -rf build'

CHTML_BUILDDIR = 'build/chtml'
TDB_BUILDDIR = 'build/tdb'
WORKS = []
WORK_MAP = {}
PAGEWORKS = []

# run LIBMPQ.
include FileUtils::Verbose
HOME_DIR = pwd
FileUtils.cd '../wowfoot-ex'
require './libs.rb'
LIBMPQ
BLP
SQUISH
PALBMP
CRBLIB
Works.run(false)
FileUtils.cd HOME_DIR

if(true)#HOST == :win32)
def rootSendSignal(works)
	puts "Signalling (#{works})..."
	res = Net::HTTP.get_response('localhost', '/unload=' + works, 3002)
	p res
	p res.body
end

def findPrerequisites(main, level=0)
	a = []
	WORKS.each do |w| w.prerequisites.each do |pre|
		next unless(pre.to_s.endsWith(DLL_FILE_ENDING))
		name = File.basename(pre.to_s, DLL_FILE_ENDING)
		pre.prerequisites.each do |p|
			next unless(p.to_s.endsWith(DLL_FILE_ENDING))
			pn = File.basename(p.to_s, DLL_FILE_ENDING)
			if(pn == main)
				a << name
				puts 'Found: ' + main + ' in ' + (' '*level) + name
				a += findPrerequisites(name, level + 1)
			end
		end
	end; end
	return a
end

# force server to unload DLLs,
# to remove the write protection.
class DllWork
	def sendSignal
		if(HOST != :win32)
			#setNewName	# causes crash
			return
		end
		# send a signal containing all the
		# idHandlers that depends on this DLL (including itself).
		# wait for a "finished" signal from the server.
		main = File.basename(self.to_s, DLL_FILE_ENDING)

		idHandlerWorks = findPrerequisites(main)
		#if(@work.respond_to?(:newDllName) && HOST == :linux)
		if(false)
			setNewName
			idHandlerWorks << (main + ':' + @NAME)
		end
		idHandlerWorks.uniq!
		rootSendSignal(idHandlerWorks.join(',')) if(!idHandlerWorks.empty?)
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
		c = writeCount(c) if(p_needed?(false))
		if(c != 0)
			@originalName = @NAME if(!@originalName)
			@NAME = newDllName(c)
			#puts "#{@originalName} is loaded. renaming to #{@NAME}"
		end
	end
	def isLoaded; if(HOST == :win32)
		# try to remove target file.
		# if that fails and the file still exists, it's loaded.
		FileUtils.rm_f(@NAME)
		return File.exist?(@NAME)
	else	# unix-type systems
		if(self.is_a?(PageWork))
			#puts "#{@NAME} is a PageWork."
			return false
		end
		#puts self
		# read /proc to find our server process.
		# read /proc/*/maps to list the loaded objects.
		#p WFC.target.to_s
		Dir['/proc/*'].each do |proc|
			#p proc
			# Can be optimized by caching the exe search result.
			# Seems fast enough for now.
			if(File.exists?(proc+'/exe')); if(File.readlink(proc+'/exe') == WFC.to_s)
				#puts "Found #{proc}"
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

	alias_method :old_execute, :fileExecute
	def fileExecute
		sendSignal if(isLoaded)
		FileUtils.rm_f(@NAME)
		raise "Loaded file was not deleted!" if(File.exist?(@NAME))
		old_execute
		#cp(@NAME, @originalName) if(@originalName)
	end
end
end	# WIN32

MemoryGeneratedFileTask.new('build/wowVersion.h') do
	@prerequisites = [DirTask.new('build')]
	io = StringIO.new
	io.puts "#ifndef WOWVERSION_H"
	io.puts "#define WOWVERSION_H"
	io.puts
	io.puts "#define CONFIG_WOW_VERSION #{CONFIG_WOW_VERSION}"
	io.puts
	io.puts "#endif\t//WOWVERSION_H"
	@buf = io.string
end

# need to run wowVersion before anything else,
# but can't have it be a prerequisite of *everything*.
Works.run(false)

COMMON = LibWork.new do
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

if(HOST == :win32)
WIN32 = DllWork.new do
	@SOURCES = ['win32']
	@EXTRA_INCLUDES = ['win32']
	@SPECIFIC_CFLAGS = {
		'win32.cpp' => ' -Wno-missing-format-attribute',
	}
	@NAME = 'win32'
end
end

class CCompileWork
	def convertHandlerDeps(deps)
		@LOCAL_DLLS ||= []
		@REQUIREMENTS ||= []
		deps.each do |dll|
			raise "Unknown work #{dll}" if(!WORK_MAP[dll])
			w = WORK_MAP[dll]
			raise "Work #{dll} doesn't exist yet!" if(!w)
			@EXTRA_INCLUDES << "handlers/#{dll}"
			f ="build/#{dll}"
			@EXTRA_INCLUDES << f if(w.is_a?(DbcWork))
			f ="#{TDB_BUILDDIR}/#{dll}"
			@EXTRA_INCLUDES << f if(w.is_a?(TdbWork))
			@LOCAL_DLLS << w
			@REQUIREMENTS << w
		end
	end
end

class HandlerWork < DllWork
	def initialize(name, handlerDeps = [], isPage = false, options = {}, &block)
		hasChtml = false
		@SOURCE_TASKS = Dir["handlers/#{name}/*.chtml"].collect do |chtml|
			hasChtml = true
			bn = File.basename(chtml, '.chtml')
			ChtmlCompileTask.new(CHTML_BUILDDIR, bn, chtml, isPage, options)
		end
		@SOURCES = ["handlers/#{name}"]
		@SOURCE_FILES = []
		@EXTRA_INCLUDES = [
			'handlers', "handlers/#{name}",
			'.', CHTML_BUILDDIR,
			'win32',
			'build',
		] + CONFIG_LOCAL_INCLUDES
		@EXTRA_INCLUDES << "#{TDB_BUILDDIR}/#{name}" if(self.is_a?(TdbWork))
		convertHandlerDeps(handlerDeps)
		@LOCAL_LIBS = [COMMON]
		@LIBRARIES = []
		@LIBRARIES << 'imagehlp' if(HOST == :win32)
		@LIBRARIES << 'dl' if(HOST != :win32)
		@LOCAL_DLLS << WIN32 if(HOST == :win32)
		@EXTRA_LINKFLAGS ||= ''
		@EXTRA_LINKFLAGS += CONFIG_LOCAL_LIBDIRS
		@NAME = name
		WORKS << self
		WORK_MAP[name] = self
		super(&block)
	end
	#def name; File.basename(@TARGET.to_s, '.so'); end
	def baseName; "#{@BUILDDIR}#{@NAME}"; end
	if(HOST != :win32) then def setup
		# TODO: fix this function, which is never called anymore
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
	def initialize(name, handlerDeps = [], &block)
		super(name, ['tdb'] + handlerDeps) do
			@SOURCE_TASKS << TdbCppTask.new(name)
			@EXTRA_CPPFLAGS = ' -Wno-invalid-offsetof'
			@REQUIREMENTS = [
				TdbStructHeaderTask.new(name),
				TdbFormatHeaderTask.new(name),
				TdbExtHeaderTask.new(name),
			]
			instance_eval(&block) if(block)
		end
	end
end

class PageWork < HandlerWork
	def initialize(name, handlerDeps = [], options = {}, &block)
		@EXTRA_LINKFLAGS = ' -u _cleanup' if(HOST == :win32)
		@EXTRA_LINKFLAGS = ' -u cleanup' if(HOST != :win32)
		PAGEWORKS << self
		super(name, ['pageContext'] + handlerDeps, true, options, &block)
	end
end

class ExTemplateWork < HandlerWork
	def initialize(name, singular, plural, upperCase, handlerDeps = [], &block)
		super(name, handlerDeps) do
			@EXTRA_INCLUDES << "build/#{name}"
			@SOURCE_TASKS << ExTemplateCpp.new(name, singular, plural, upperCase)
			@SOURCE_FILES << "../wowfoot-ex/output/#{name}.data.cpp"
			instance_eval(&block) if(block)
		end
	end
end

class DbcWork < HandlerWork
	def initialize(name, handlerDeps = [], &block)
		handlerDeps << 'dbc'
		super(name, handlerDeps) do
			@EXTRA_INCLUDES << '../wowfoot-ex/src/libs'
			@EXTRA_INCLUDES << "build/#{name}"
			@SOURCE_TASKS ||= []
			@SOURCE_TASKS << DbcCppTask.new(name)
			instance_eval(&block) if(block)
		end
	end
end

# HandlerWorks
HandlerWork.new('areaMap')

TDB = HandlerWork.new('tdb') do
	@EXTRA_CPPFLAGS = ' -Wno-shadow -Wno-attributes'	# mysql++ header bugs
	@LIBRARIES << 'mysqlclient'
	if(HOST == :win32)
		@EXTRA_INCLUDES += CONFIG_MYSQL_INCLUDES
		@EXTRA_LINKFLAGS += CONFIG_MYSQL_LIBDIRS
		@LIBRARIES << 'ws2_32'
	else
		@EXTRA_INCLUDES << '/usr/include/mysql'
	end
end

DBC = HandlerWork.new('dbc') do
	@SOURCES << '../wowfoot-ex/src/libs'
	@SOURCE_FILES << '../wowfoot-ex/src/dbcList.cpp'
	@EXTRA_INCLUDES << '../wowfoot-ex/src'
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs/libmpq'
	@SPECIFIC_CFLAGS = {
		'loadlib.cpp' => ' -Wno-multichar',
		'dbcList.cpp' => " -DCONFIG_WOW_VERSION=#{CONFIG_WOW_VERSION}",
	}
	set_defaults
	@EXTRA_OBJECTS = [LIBMPQ]
end

HandlerWork.new('dbcItemSubClass', ['dbc']) do
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs'
end
HandlerWork.new('dbcItemClass', ['dbc']) do
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs'
end
HandlerWork.new('icon', ['dbc']) do
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs'
	@EXTRA_INCLUDES << '../wowfoot-ex/src/libs/libmpq'
	@prerequisites ||= []
	@prerequisites << DirTask.new('build/icon')
	@EXTRA_OBJECTS = [(LIBMPQ), (BLP), (SQUISH), (PALBMP), (CRBLIB)]
	@LIBRARIES += ['png', 'jpeg']
end

if(CONFIG_WOW_VERSION > 30000)
TdbWork.new('db_achievement_reward')
end
TdbWork.new('db_quest')
TdbWork.new('db_loot_template')
TdbWork.new('db_item')
TdbWork.new('db_npc_vendor')
TdbWork.new('db_npc_trainer')
TdbWork.new('db_spawn')
TdbWork.new('db_creature_template', ['db_spawn'])
TdbWork.new('db_gameobject_template', ['db_spawn'])
TdbWork.new('db_creature_onkill_reputation')

DbcWork.new('dbcFactionTemplate')
DbcWork.new('dbcSpellIcon')
DbcWork.new('dbcItemDisplayInfo')
DbcWork.new('dbcItemSet')
DbcWork.new('dbcFaction')
DbcWork.new('dbcArea')
DbcWork.new('dbcSpell')
DbcWork.new('dbcWorldMapArea')
if(CONFIG_WOW_VERSION > 30000)
DbcWork.new('dbcQuestFactionReward')
DbcWork.new('dbcCharTitles')
DbcWork.new('dbcTotemCategory')
DbcWork.new('dbcAchievement')
DbcWork.new('dbcItemExtendedCost', ['db_npc_vendor', 'db_creature_template', 'db_item'])
DBC_ACHIEVEMENT_COND = ['dbcAchievement']
DBC_QFR_COND = ['dbcQuestFactionReward']
DBC_TOTEM_CATEGORY_COND = ['dbcTotemCategory']
DBC_CHAR_TITLES_COND = ['dbcCharTitles']
DBC_ITEM_EXTENDED_COST_COND = ['dbcItemExtendedCost']
else
DBC_ACHIEVEMENT_COND = []
DBC_QFR_COND = []
DBC_TOTEM_CATEGORY_COND = []
DBC_CHAR_TITLES_COND = []
DBC_ITEM_EXTENDED_COST_COND = []
end
DbcWork.new('dbcChrClasses')
DbcWork.new('dbcChrRaces')
#DbcWork.new('dbcSpellEffectNames')	# missing from mpqSet. TODO: investigate.
DbcWork.new('dbcSpellMechanic')
DbcWork.new('dbcSkillLineAbility')
DbcWork.new('dbcSkillLine')
DbcWork.new('dbcSkillLineCategory')
DbcWork.new('dbcLock', ['db_creature_template', 'db_gameobject_template', 'db_item'])

HandlerWork.new('pageContext')
HandlerWork.new('tabs')
HandlerWork.new('tabTable', ['tabs'])
HandlerWork.new('mapSize')
HandlerWork.new('skillShared', ['dbcLock', 'dbcSkillLine'])
HandlerWork.new('questShared', ['db_quest'])

class PatchTask < FileTask
	def initialize
		@src = 'handlers/comments/patch.rb'
		@prerequisites = [FileTask.new(@src)]
		super('build/patch.cpp')
	end
	def fileExecute
		sh "ruby #{@src} #{@NAME}"
	end
end

commentDeps = ['tabs', 'dbcSpell', 'db_item', 'dbcWorldMapArea',
	'db_quest', 'db_creature_template',
	'db_gameobject_template', 'dbcFaction',
	'questShared',
] + DBC_ACHIEVEMENT_COND
HandlerWork.new('comments', commentDeps) do
	@LIBRARIES << 'sqlite3'
	@SOURCE_TASKS << PatchTask.new
end

HandlerWork.new('spawnPoints', ['mapSize', 'dbcArea', 'dbcWorldMapArea', 'areaMap', 'db_spawn'])

TdbWork.new('db_questrelation', ['tabs', 'tabTable', 'db_quest', 'db_spawn',
	'db_creature_template', 'db_gameobject_template', 'spawnPoints', 'mapSize'])

PageWork.new('comment', ['tabTable', 'tabs', 'comments'])
PageWork.new('quests', ['tabTable', 'tabs', 'db_quest', 'dbcFaction'],
	{:constructor => true})
PageWork.new('quest', ['tabTable', 'tabs', 'comments', 'db_quest', 'dbcSpell', 'db_creature_template',
	'db_item', 'dbcFaction', 'db_questrelation', 'db_gameobject_template'] + DBC_QFR_COND) do
	@EXTRA_CPPFLAGS = ' -save-temps=obj'
end
if(CONFIG_WOW_VERSION > 30000)
PageWork.new('title', ['dbcAchievement', 'tabTable', 'tabs', 'comments', 'dbcCharTitles',
	'db_achievement_reward'])
PageWork.new('achievement', ['dbcAchievement', 'tabs', 'comments', 'dbcCharTitles',
	'db_item', 'db_creature_template', 'db_achievement_reward'])
end
PageWork.new('npc', ['db_creature_template',
	'db_spawn', 'tabs', 'comments', 'spawnPoints', 'mapSize',
	'dbcFaction', 'dbcFactionTemplate', 'db_questrelation'])
PageWork.new('zone', ['dbcArea', 'dbcWorldMapArea', 'mapSize', 'tabs', 'spawnPoints',
	'db_questrelation', 'db_spawn'])
PageWork.new('search', ['dbcArea', 'dbcWorldMapArea', 'tabs', 'tabTable', 'dbcSpell', 'db_item',
	'db_creature_template', 'db_gameobject_template', 'db_quest',
	'dbcItemSet', 'dbcFaction'] + DBC_ACHIEVEMENT_COND + DBC_CHAR_TITLES_COND)
PageWork.new('item', ['tabs', 'tabTable', 'db_item', 'comments',
	'db_npc_vendor', 'db_creature_template', 'dbcSpell',
	'db_loot_template', 'dbcChrClasses', 'dbcChrRaces', 'db_gameobject_template',
	'dbcItemClass', 'dbcItemSubClass', 'dbcItemSet', 'icon', 'dbcItemDisplayInfo',
	'dbcSkillLine', 'questShared',
	'db_questrelation', 'db_quest'] + DBC_TOTEM_CATEGORY_COND + DBC_ITEM_EXTENDED_COST_COND)
PageWork.new('faction', ['tabTable', 'tabs', 'comments', 'dbcFaction', 'item',
	'db_quest', 'db_creature_template', 'dbcFactionTemplate',
	'db_creature_onkill_reputation'])
PageWork.new('itemset', ['tabs', 'tabTable', 'db_item', 'comments',
	'db_npc_vendor', 'db_creature_template', 'dbcSpell',
	'db_loot_template', 'dbcChrClasses', 'dbcChrRaces', 'db_gameobject_template',
	'dbcItemClass', 'dbcItemSubClass', 'dbcItemSet', 'item'] +
	DBC_TOTEM_CATEGORY_COND + DBC_ITEM_EXTENDED_COST_COND)
PageWork.new('object', ['db_gameobject_template',
	'db_spawn', 'tabs', 'comments', 'spawnPoints', 'mapSize', 'item', 'tabTable',
	'db_loot_template', 'db_item', 'db_questrelation'])
PageWork.new('spell', ['tabs', 'tabTable', 'db_item', 'comments', 'dbcSpell',
	'db_creature_template', 'dbcSpellIcon', 'icon',
	#'dbcSpellEffectNames',
	'dbcSpellMechanic',
	'dbcSkillLineAbility',
	])
PageWork.new('spells', ['tabs', 'tabTable', 'dbcSpell',
	'dbcItemDisplayInfo', 'db_item',
	'dbcSpellIcon', 'icon',
	'dbcSkillLineAbility', 'dbcSkillLine',
	'db_npc_trainer', 'db_creature_template',
	'skillShared',
	], {:constructor => true})
PageWork.new('items', ['db_item', 'dbcItemClass', 'dbcItemSubClass', 'item', 'tabTable', 'tabs'],
	{:constructor => true})
PageWork.new('skills', ['tabs', 'tabTable', 'dbcSkillLine', 'dbcSkillLineCategory',
	'dbcSpellIcon', 'icon'])
PageWork.new('skill', ['tabTable', 'tabs', 'comments', 'dbcLock', 'dbcSpell', 'icon',
	'dbcItemDisplayInfo', 'dbcSkillLineAbility', 'dbcSkillLine', 'skillShared',
	'db_gameobject_template', 'db_item', 'db_creature_template'],
	{:constructor => true})

WFC = @wfc = ExeWork.new do
	@SOURCES = ['.']
	@LIBRARIES = ['microhttpd']
	@EXTRA_INCLUDES = ['win32', '.', 'build']#'src', 'src/libs/libmpq']
	#@EXTRA_CFLAGS = ' -D_POSIX_SOURCE'	#avoid silly bsd functions
	@LOCAL_LIBS = [COMMON]
	@LOCAL_DLLS = []

	if(HOST == :win32)
		@SOURCE_FILES = ["dll/dll-win32.cpp"]
		@LIBRARIES << 'wsock32'
		@LIBRARIES << 'imagehlp'
		@LOCAL_DLLS << WIN32
	elsif(HOST == :linux || HOST == :darwin)
		@SOURCE_FILES = ["dll/dll-unix.cpp"]
		@LIBRARIES << 'dl'
	else
		error "Unsupported platform"
	end

	@NAME = 'wowfoot-cpp'

	def buildDir; @BUILDDIR; end
end

TestHandlerDeps = [
	'db_quest',
	'db_item',
	'db_creature_template',
	'db_gameobject_template',
	'dbcItemSet',
	'dbcFaction',
	'dbcWorldMapArea',
	'dbcSpell',
] + DBC_ACHIEVEMENT_COND + DBC_CHAR_TITLES_COND

TEST = ExeWork.new do
	@SOURCES = ['test']
	@EXTRA_INCLUDES = ['.', 'handlers'] + CONFIG_LOCAL_INCLUDES
	@LIBRARIES = ['tidy']
	if(HOST == :win32)
		@LIBRARIES << 'curldll'
	else
		@LIBRARIES << 'curl'
	end
	convertHandlerDeps(TestHandlerDeps)
	@EXTRA_CPPFLAGS = ' -fopenmp'
	@EXTRA_LINKFLAGS = ' -fopenmp -Wl,-rpath,.' + CONFIG_LOCAL_LIBDIRS

	@NAME = 'wowfoot-test'
	def run; sh @TARGET; end
end

QUEST_ANALYZER = ExeWork.new do
	@SOURCE_FILES = ['tools/questAnalyzer.cpp']
	@EXTRA_INCLUDES = ['.', 'handlers'] + CONFIG_LOCAL_INCLUDES
	convertHandlerDeps(TestHandlerDeps + [
		'db_questrelation',
		'db_spawn',
	])
	@LOCAL_DLLS << COMMON
	if(HOST == :win32)
		@LIBRARIES = ['imagehlp']
		@LOCAL_DLLS << WIN32
	end
	@NAME = 'quest-analyzer'
end

def cmd; "#{@wfc} #{@wfc.buildDir}"; end

target :default do
end

target :run => :default do
	rm_f 'build/count'
	rm_f Dir.glob('build/*/*.count')
	#startWebBrowser("http://localhost:#{3002}/#{CONFIG_AUTOSTART_PATH}")
	sh cmd
end

target :test do
	TEST.run
end

target :qa do
	sh QUEST_ANALYZER.to_s
	cd 'build'
	sh 'dot -Tsvg quests.dot > quests.svg'
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

Works.run
