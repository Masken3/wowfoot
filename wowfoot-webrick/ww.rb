#!/usr/bin/ruby

require 'webrick'
include WEBrick
require 'erb'
require '../wowfoot-ex/output/WorldMapArea.rb'
require '../wowfoot-ex/output/AreaTable.rb'
#require './tdb.rb'
require 'dbi'

S = HTTPServer.new( :Port => 3001 )#, :DocumentRoot => File.dirname(__FILE__) + "/htdocs" )

module TDB
	C = DBI::connect('dbi:Mysql:world', 'trinity', 'trinity')
end

def run(path, bind = binding)
	eval(open(path).read, bind, path)
end

class MyPageServlet < HTTPServlet::AbstractServlet
	def initialize(path)
		@path = path
	end
	def get_instance(a)
		return self
	end
	def do_GET(req, response)
		File.open("htdocs#{@path}.rhtml",'r') do |f|
			@template = ERB.new(f.read)
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
		@template.result(binding())
	end
end

class IdClassServlet < HTTPServlet::AbstractServlet
	@@table = {}
	def self.mount(path, servletClass)
		puts "Mount: #{path}"
		@@table[path] = servletClass
	end
	def do_GET(req, response)
		path = req.path.match("([a-z/]+)=")[1]
		#p path
		sc = @@table[path]
		if(!sc)
			raise HTTPStatus[404], "`#{req.path}' not found."
		end
		sc.do_GET(req, response)
	end
end

# requires the following functions to be defined by its subclasses:
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
mountTextIdPage('search')

S.mount('/', IdClassServlet)
S.mount('/output', HTTPServlet::FileHandler, 'htdocs/output')

trap("INT"){
  S.shutdown
}
S.start
