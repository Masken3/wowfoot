#!/usr/bin/ruby

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/exe.rb'
require File.expand_path '../rules/dll.rb'

wfe = ExeWork.new
wfe.instance_eval do
	@SOURCES = ['.']
	#@EXTRA_INCLUDES = ['src', 'src/libs/libmpq']
	@IGNORED_FILES = []
	#@EXTRA_CFLAGS = ' -D_POSIX_SOURCE'	#avoid silly bsd functions
	@SPECIFIC_CFLAGS = {
	}
	@LIBRARIES = ['microhttpd']

	@NAME = 'wowfoot-cpp'
end

wfe.invoke
