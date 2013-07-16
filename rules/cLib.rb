require "#{File.dirname(__FILE__)}/cCompile.rb"
require "#{File.dirname(__FILE__)}/host.rb"

class LibWork < CCompileWork
	def cFlags
		return @cFlags if(@cFlags)
		return @cFlags = libCmd
	end
	def fileExecute
		execFlags
		preLib
		sh cFlags
		postLib
	end
	def targetName()
		return CCompileTask.genFilename(@BUILDDIR, @NAME, HOST_LIB_FILE_ENDING)
	end
end
