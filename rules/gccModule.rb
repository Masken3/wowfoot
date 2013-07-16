require "#{File.dirname(__FILE__)}/loader_md.rb"
require "#{File.dirname(__FILE__)}/cCompile.rb"
require "#{File.dirname(__FILE__)}/gccFlags.rb"


def get_gcc_version_info(gcc)
	info = {}
	open("|#{gcc} -v 2>&1") do |file|
		file.each do |line|
			parts = line.split(/ /)
			#puts "yo: #{parts.inspect}"
			if(parts[0] == 'Target:' && parts[1].strip == 'arm-elf')
				info[:arm] = true
			end
			if(parts[0] == "gcc" && parts[1] == "version")
				info[:ver] = parts[2].strip
			elsif(parts[0] == 'clang' && parts[1] == 'version')
				info[:clang] = true
				info[:ver] = parts[2].strip
			end
		end
	end
	if(!info[:ver])
		puts gcc
		error("Could not find gcc version.")
	end
	info[:string] = ''
	info[:string] << 'arm-' if(info[:arm])
	info[:string] << 'clang-' if(info[:clang])
	info[:string] << info[:ver]
	return info
end

module GccCompilerModule
	include GccFlags

	def gcc
		'gcc'
	end

	def objFileEnding
		'.o'
	end

	def builddir_postfix
		if(USE_COMPILER_VERSION_IN_BUILDDIR_NAME)
			return '-' + gccVersionInfo[:string]
		else
			return ''
		end
	end

	def loadDependencies
		if(!File.exists?(@DEPFILE))
			@needed = "Because the dependency file is missing:"
			return []
		end
		MakeDependLoader.load(@DEPFILE, @NAME)
	end

private
	@@gcc_info = {}
	def gccVersionInfo
		if(!@@gcc_info[gcc])
			@@gcc_info[gcc] = get_gcc_version_info(gcc)
		end
		return @@gcc_info[gcc]
	end

	def setGccVersion
		info = gccVersionInfo

		@GCC_IS_V4 = info[:ver][0] == "4"[0]
		if(@GCC_IS_V4)
			@GCC_V4_SUB = info[:ver][2, 1].to_i
		end

		# Assuming for the moment that clang is command-line-compatible with gcc 4.2.
		@GCC_IS_CLANG = info[:clang]
		if(@GCC_IS_CLANG)
			@GCC_IS_V4 = true
			@GCC_V4_SUB = 2
		end
	end

public

	# used only by CCompileWork.
	def loadCommonFlags
		setGccVersion
		@TARGET_PLATFORM = HOST

		define_cflags

		@CFLAGS_MAP = {
			'.c' => @CFLAGS,
			'.cpp' => @CPPFLAGS,
			'.cc' => @CPPFLAGS,
			'.C' => @CPPFLAGS,
			'.s' => ' -Wa,--gstabs',
		}
	end

	def compileCmd
		flags = @FLAGS

		@TEMPDEPFILE = @DEPFILE + 't'
		flags += " -MMD -MF \"#{@TEMPDEPFILE}\""

		return "#{gcc} -o \"#{@NAME}\"#{flags} -c \"#{File.expand_path_fix(@SOURCE)}\""
	end

	def postCompile
		# In certain rare cases (error during preprocess caused by a header file)
		# gcc may output an empty dependency file, resulting in an empty dependency list for
		# the object file, which means it will not be recompiled, even though it should be.
		# Therefore, we only write the real depFile after successful compilation.
		FileUtils.mv(@TEMPDEPFILE, @DEPFILE)
	end

	def linkCmd
		linkerName = @cppfiles.empty? ? 'gcc' : 'g++'
		flags = @EXTRA_LINKFLAGS
		@LIBRARIES.each do |lib|
			flags += " -l#{lib}"
		end
		raise hell if(@LIBRARIES.uniq.length != @LIBRARIES.length)
		raise hell if(@object_tasks.uniq.length != @object_tasks.length)
		return "#{linkerName} -o \"#{@NAME}\"#{@FLAGS} \"#{@object_tasks.join("\" \"")}\"#{flags}"
	end

	def dllCmd
		raise hell if(@FLAGS)
		@FLAGS = ' -shared'
		return linkCmd
	end

	def postLink
	end

	def preLib
		# ar does not remove out-of-date archive members.
		# The file must be deleted if we are to get a clean build.
		FileUtils.rm_f(@NAME)
	end

	def libCmd
		return "ar rcs #{@NAME} #{@FLAGS} \"#{@object_tasks.join("\" \"")}\""
	end

	def postLib
	end
end
