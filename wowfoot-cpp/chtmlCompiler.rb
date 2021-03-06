require File.expand_path '../rules/work.rb'
require 'timeout'

class ChtmlCompileTask < MultiFileTask
	def initialize(builddir, name, src, isPage, options)
		@name = name
		@cpp = "#{builddir}/#{name}.chtml.cpp"
		@header = "#{builddir}/#{name}.chtml.h"
		# todo: a change to any of these variable is cause for rebuild.
		@src = src
		@isPage = isPage
		@options = options

		@prerequisites ||= []
		@prerequisites << DirTask.new(builddir)
		@prerequisites << FileTask.new(@src)
		@prerequisites << FileTask.new(__FILE__)
		super(@cpp, [@header])
	end
	def fileExecute
		Timeout::timeout(5) do
			compile
		end
	end
	private
	def compile
		src = File.new(@src, 'r')
		cpp = File.new(@cpp, 'w')
		header = File.new(@header, 'w')
		header << "#include \"#{@name}.h\"\n"
		header << "#include \"chtmlUtil.h\"\n"

		@state = :norm
		cpp << "#include <#{@name}.chtml.h>\n"
		cpp << "\n"
		dllexport = ' __declspec(dllexport)' if(HOST == :win32)
		if(@isPage)
			if(!@options[:constructor])
				cpp << "#{@name}Chtml::#{@name}Chtml() : PageContext(\"#{@name.capitalize}\") {}\n"
				cpp << "\n"
			end
			cpp << "extern \"C\"\n"
			cpp << "void#{dllexport} getResponse(const char* urlPart, DllResponseData* drd) {\n"
			cpp << "	#{@name}Chtml context;\n"
			cpp << "	getResponse(urlPart, drd, context);\n"
			cpp << "}\n"
			cpp << "\n"
		end
		runFunctionName = @isPage ? 'runPage' : 'run';
		cpp << "int#{dllexport} #{@name}Chtml::#{runFunctionName}(ostream& stream) {\n"
		cpp << "int returnCode = 200;\n"
		cpp << "# 1 \"#{File.expand_path(@src)}\"\n"

		chtml = src.read
		pos = 0
		lineStart = 0
		status = :idle
		while(pos < chtml.length)
			if(chtml[pos,1] == '%' && pos == lineStart)	# C++ line
				cpp << ";\n" if(status == :cdata)
				pos += 1
				eol = chtml.index("\n", pos)
				if(chtml[pos,1] == '=')	# stream C++ expression
					pos += 1
					cpp << 'stream <<'<<chtml[pos..eol-1]<<";\n"
				else	# C++ code
					# skip whitespace
					while(chtml[pos,1] == ' ')
						pos += 1
					end
					cpp << chtml[pos..eol]
				end
				pos = eol+1
				lineStart = pos
				status = :idle
			elsif(chtml[pos,2] == '<%')	# C++ code
				cpp << "\n" if(pos == lineStart && status != :idle)
				pos += 2
				raise hell if(status == :exp || status == :code)
				prevStatus = status
				if(chtml[pos,1] == '=')	# stream C++ expression
					pos += 1
					cpp << 'stream <<' if(status == :idle)
					if(status == :cdata)
						cpp << '"' unless(pos == lineStart+3)
						cpp << '<<'
					end
					status = :exp
				else
					if(status == :cdata)
						cpp << '"' unless(pos == lineStart+2)
						cpp << ';'
					end
					status = :code
				end
			elsif(chtml[pos,2] == '%>')
				pos += 2
				if(status == :exp)
				elsif(status == :code)
				else
					raise hell
				end
				if(prevStatus == :cdata)
					if(status == :exp)
						cpp << '<<"'
						status = :cdata
					else
						status = :idle
					end
				else
					cpp << ';' if(status == :exp)
					status = :idle
				end
			else
				# character data. handle special characters.
				if(status == :cdata && pos == lineStart)
					cpp << "\n\""
				elsif(status == :idle)
					cpp << 'stream << "'
					status = :cdata
				end
				s = chtml[pos,1]
				if(s == "\n" && status == :cdata)
					cpp << '\n"'
					lineStart = pos+1
				elsif(s == '"' && status == :cdata)
					cpp << '\"'
				else
					cpp << chtml[pos,1]
				end
				pos += 1
			end
		end

		cpp << ';'<<"\n" if(status == :cdata)
		cpp << "return returnCode;\n"
		cpp << "}\n"
		cpp.close
		header.close
	end
	def hackEmbedded(line)
		if(line == '<%')
			@state = :execute
			return false
		elsif(line == '<%=')
			@state = :stream
			return false
		end
		if(@state == :norm)
			index = line.index('<%')
			return line if(!index)
		end
	end
end
