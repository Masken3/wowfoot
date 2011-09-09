#!/usr/bin/ruby

STDOUT.sync = true
puts 'Loading...'
START_TIME = Time.now
#def hell
#	'hell'
#end

require 'webrick'
include WEBrick
require 'erb'
#require './tdb.rb'
require 'dbi'
require './src/config.rb'
require './src/dbiExtras.rb'
require './src/util.rb'

require './src/itemClass.rb'
require './src/itemEquip.rb'
require './src/itemQuality.rb'
require './src/itemMaterial.rb'
require './src/itemFlags.rb'
require './src/itemStat.rb'
require './src/itemEnums.rb'

S = HTTPServer.new( :Port => 3001 )

module TDB	# Trinity DataBase
	C = DBI::connect('dbi:Mysql:'+TDB_DATABASE, TDB_USER, TDB_PASSWORD)
	DBI::setupExtras(C)
end

module CDB	# Comment DataBase
	C = DBI::connect('dbi:SQLite3:../wowfoot-import/imports.db')
	DBI::setupExtras(C)
end

require '../wowfoot-ex/output/Map.rb'
require '../wowfoot-ex/output/WorldMapContinent.rb'
require '../wowfoot-ex/output/WorldMapArea.rb'
require '../wowfoot-ex/output/AreaTable.rb'
require '../wowfoot-ex/output/QuestSort.rb'
require '../wowfoot-ex/output/Faction.rb'
require '../wowfoot-ex/output/QuestFactionReward.rb'
require '../wowfoot-ex/output/TotemCategory.rb'
require '../wowfoot-ex/output/ItemExtendedCost.rb'
require './src/areaMap.rb'
require './src/coordinates.rb'
require './src/itemCurrency.rb'

require './src/referenceLoot.rb'

def run(path, bind = binding)
	path = File.expand_path('src/'+path)
	eval(open(path).read, bind, path)
end

def rhtml(path, bind = binding)
	path = File.expand_path("src/#{path}.rhtml")
	template = nil
	File.open(path,'r') do |f|
		template = ERB.new(f.read)
		template.setFile(path)
	end
	return template.result(bind)
end

class ERB
	def setFile(fname)
		@filename = fname
	end
end

class MyPageServlet < HTTPServlet::AbstractServlet
	def initialize(path)
		@path = path
	end
	def get_instance(a)
		return self
	end
	def do_GET(req, response)
		path = "htdocs#{@path}.rhtml"
		File.open(path,'r') do |f|
			@template = ERB.new(f.read)
			@template.setFile(path)
		end
		response.body = getBody(req)
		response['Content-Type'] = "text/html"
	end
end

class SinglePageServlet < MyPageServlet
	def getBody(req)
		if(req.path != @path)
			raise HTTPStatus[404], "`#{req.path}' not found."
		end
		@template.result
	end
end

class IdStruct
	def initialize(id)
		@id = id
	end
	def bind
		id = @id
		return binding
	end
end

class IdPageServlet < MyPageServlet
	def initialize(path, pattern)
		super(path)
		@pattern = "#{@path}=" + pattern
	end
	def getBody(req)
		#p @pattern
		id = req.path.match(@pattern)
		#p id
		if(!id)
			raise HTTPStatus[404], "`#{req.path}' not found."
		end
		@template.result(IdStruct.new(id).bind)
	end
end

class IdClassServlet < HTTPServlet::AbstractServlet
	@@table = {}
	def self.mount(path, servletClass)
		puts "Mount: #{path}"
		@@table[path] = servletClass
	end
	def do_GET(req, response)
		match = req.path.match("([a-z/]+)=")
		path = match[1] if(match)
		#p path
		sc = @@table[path]
		if(!sc)
			raise HTTPStatus[404], "`#{req.path}' not found."
		end
		sc.do_GET(req, response)
	end
end

def mountSinglePage(name)
	path = "/#{name}"
	S.mount(path, SinglePageServlet.new(path))
end

def mountIdPage(name)
	path = "/#{name}"
	pattern = "([0-9]+)"
	IdClassServlet.mount(path, IdPageServlet.new(path, pattern))
end

def mountTextIdPage(name)
	path = "/#{name}"
	pattern = "(.+)"
	IdClassServlet.mount(path, IdPageServlet.new(path, pattern))
end

mountSinglePage('areas')
mountIdPage('area')
mountIdPage('npc')
mountIdPage('item')
mountIdPage('quest')
mountIdPage('object')
mountTextIdPage('search')

S.mount('/', IdClassServlet)
S.mount('/output', HTTPServlet::FileHandler, '../wowfoot-ex/output')
S.mount('/static', HTTPServlet::FileHandler, 'htdocs/static')

trap("INT") {
  S.shutdown
}
puts "Ready to start. #{Time.now - START_TIME} seconds elapsed."
STDOUT.flush
S.start
