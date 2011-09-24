#!/usr/bin/ruby

require File.expand_path '../rules/host.rb'
require File.expand_path '../rules/exe.rb'
require File.expand_path '../rules/dll.rb'

zone = DllWork.new
zone.instance_eval do
	@EXTRA_SOURCEFILES = ['handlers/zone.cpp']
	@EXTRA_INCLUDES = ['.']
	@NAME = 'zone'
end

@wfc = ExeWork.new
@wfc.instance_eval do
	@SOURCES = ['.']
	@LIBRARIES = ['microhttpd']
	#@EXTRA_INCLUDES = ['src', 'src/libs/libmpq']
	@IGNORED_FILES = []
	#@EXTRA_CFLAGS = ' -D_POSIX_SOURCE'	#avoid silly bsd functions
	@SPECIFIC_CFLAGS = {
	}

	if(HOST == :win32)
		@EXTRA_SOURCEFILES = ["dll/dll-win32.cpp"]
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
	@wfc.invoke
	zone.invoke
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
