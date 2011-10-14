
def sql_pair(type, array, count)
	a = []
	(1..count).each do |i|
		array.each do |c|
			a << [type, "#{c}#{i}"]
		end
	end
	return a
end

class ERB
	def setFile(fname)
		@filename = fname
	end
end

# escape C++ keywords
def cEscape(name)
	return 'class_' if(name.to_s == 'class')
	return name
end

class TdbStructHeaderTask < MemoryGeneratedFileTask
	def initialize(work, name)
		@src = FileTask.new(self, "handlers/#{name}.rb")
		@dst = "#{TDB_BUILDDIR}/#{name}.struct.h"
		super(work, @dst)
		@prerequisites << DirTask.new(self, TDB_BUILDDIR)

		instance_eval(open(@src).read, @src)
		upName = name.upcase
		template = %q{
#ifndef <%=upName%>_STRUCT_H
#define <%=upName%>_STRUCT_H

struct <%=@structName%> {<% @struct.each do |col| %>
	<%=col[0]%> <%=cEscape(col[1])%>;<%end%>
};

#endif	//<%= upName %>_STRUCT_H
}
		@buf = ERB.new(template).result(binding)
	end
end

class TdbFormatHeaderTask < MemoryGeneratedFileTask
	FORMATS = {
		:int => 'CDT_INT',
		:string => 'CDT_STRING',
		:float => 'CDT_FLOAT',
	}
	def initialize(work, name)
		@src = FileTask.new(self, "handlers/#{name}.rb")
		@dst = "#{TDB_BUILDDIR}/#{name}.format.h"
		super(work, @dst)
		@prerequisites << DirTask.new(self, TDB_BUILDDIR)

		instance_eval(open(@src).read, @src)
		upName = name.upcase
		template = %q{
#ifndef <%=upName%>_FORMAT_H
#define <%=upName%>_FORMAT_H

static const ColumnFormat s<%=@structName%>Formats[] = {<% @struct.each do |col| %>
{<%=FORMATS[col[0]]%>, "<%=col[1]%>"},<%end%>
};

#endif	//<%= upName %>_FORMAT_H
}
		@buf = ERB.new(template).result(binding)
	end
end
