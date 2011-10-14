
def sql_pair(type, array, count)
	return [[type, array, count]]
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
		@src = FileTask.new(self, "handlers/#{name}/#{name}.rb")
		@dst = "#{TDB_BUILDDIR}/#{name}.struct.h"
		super(work, @dst)
		@prerequisites << DirTask.new(self, TDB_BUILDDIR)

		instance_eval(open(@src).read, @src)
		upName = name.upcase
		template = %q{
#ifndef <%=upName%>_STRUCT_H
#define <%=upName%>_STRUCT_H

struct <%=@structName%> {<% @struct.each do |col|
	if(col.size == 2) %>
	<%=col[0]%> <%=cEscape(col[1])%>;<%else
	col[1].each do |name| %>
	<%=col[0]%> <%=cEscape(name)%>[<%=col[2]%>];<%end; end; end%>
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
		@src = FileTask.new(self, "handlers/#{name}/#{name}.rb")
		@dst = "#{TDB_BUILDDIR}/#{name}.format.h"
		super(work, @dst)
		@prerequisites << DirTask.new(self, TDB_BUILDDIR)

		instance_eval(open(@src).read, @src)
		upName = name.upcase
		template = %q{
#ifndef <%=upName%>_FORMAT_H
#define <%=upName%>_FORMAT_H

#include <stddef.h>

static const ColumnFormat s<%=@structName%>Formats[] = {<% @struct.each do |col|
	if(col.size == 2) %>
{<%=FORMATS[col[0]]%>, "<%=col[1]%>", offsetof(<%=@structName%>, <%=cEscape(col[1])%>)},<% else
	(1..col[2]).each do |i| col[1].each do |name| %>
{<%=FORMATS[col[0]]%>, "<%=name%><%=i%>", offsetof(<%=@structName%>, <%=cEscape(name)%>[<%=i-1%>])},<%end; end; end; end%>
};

#endif	//<%= upName %>_FORMAT_H
}
		@buf = ERB.new(template).result(binding)
	end
end
