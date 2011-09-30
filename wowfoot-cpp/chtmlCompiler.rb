require File.expand_path '../rules/task.rb'

class ChtmlCompileTask < MultiFileTask
	def initialize(work, builddir, name, src)
		@name = name
		@cpp = "#{builddir}/#{name}.chtml.cpp"
		@header = "#{builddir}/#{name}.chtml.h"
		@src = src
		super(work, @cpp, @header)
		@prerequisites << DirTask.new(work, builddir)
		@prerequisites << FileTask.new(work, @src)
		@prerequisites << FileTask.new(work, __FILE__)
	end
	def execute
		src = File.new(@src, 'r')
		cpp = File.new(@cpp, 'w')
		header = File.new(@header, 'w')
		header << "#include \"chtmlUtil.h\"\n"
		header << "#include \"#{@name}.h\"\n"

		@state = :norm
		cpp << "#include \"#{@name}.chtml.h\"\n"
		cpp << "\n"
		cpp << "int #{@name}Chtml::run(ostream& stream) {\n"
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
					cpp << 'stream <<'<<chtml[pos..eol-1]<<"\n"
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
				pos += 2
				raise hell if(status == :exp || status == :code)
				prevStatus = status
				if(chtml[pos,1] == '=')	# stream C++ expression
					pos += 1
					cpp << 'stream <<' if(status == :idle)
					cpp << '"<<' if(status == :cdata)
					status = :exp
				else
					cpp << '";' if(status == :cdata)
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
					cpp << '<<"'
					status = :cdata
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
