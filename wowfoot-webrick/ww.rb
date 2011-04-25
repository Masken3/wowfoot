#!/usr/bin/ruby

require 'webrick'
include WEBrick
require 'erb'
require '../wowfoot-ex/output/WorldMapArea.rb'

s = HTTPServer.new( :Port => 3001, :DocumentRoot => File.dirname(__FILE__) + "/htdocs" )

class AreasServlet < HTTPServlet::AbstractServlet
	def do_GET(req, response)
		if(req.path != '/areas')
			raise HTTPStatus[404]
		end
		File.open('htdocs/areas.rhtml','r') do |f|
			@template = ERB.new(f.read)
		end
		response.body = @template.result
		response['Content-Type'] = "text/html"
	end
end

s.mount("/areas", AreasServlet)

trap("INT"){
  s.shutdown
}
s.start
