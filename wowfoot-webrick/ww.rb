#!/usr/bin/ruby

require 'webrick'
include WEBrick
require 'erb'
require '../wowfoot-ex/output/WorldMapArea.rb'

S = HTTPServer.new( :Port => 3001 )#, :DocumentRoot => File.dirname(__FILE__) + "/htdocs" )

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
	def getBody(req)
		#p "#{@path}=([0-9]+)"
		id = req.path.match("#{@path}=([0-9]+)")
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
		@@table[path] = servletClass
	end
	def do_GET(req, response)
		md = req.path.match("([a-z/]+)=([0-9]+)")
		path, id = md[1], md[2]
		#p path
		#p id
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
	IdClassServlet.mount(path, IdPageServlet.new(path))
end

mountSinglePage('areas')
mountIdPage('area')

S.mount('/', IdClassServlet)

trap("INT"){
  S.shutdown
}
S.start
