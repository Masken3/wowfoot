
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

class TdbGenTask < MemoryGeneratedFileTask
	FORMATS = {
		:int => 'CDT_INT',
		:string => 'CDT_STRING',
		:float => 'CDT_FLOAT',
	}
	def capitalize(foo)
		return foo[0,1].upcase + foo[1..-1]
	end
	def initialize(work, name, template, dstName)
		src = FileTask.new(self, "handlers/#{name}/#{name}.rb")
		dst = "#{TDB_BUILDDIR}/#{dstName}"
		super(work, dst)
		@prerequisites << DirTask.new(self, TDB_BUILDDIR)
		instance_eval(open(src).read, src)
		upName = name.upcase

		@types = {}
		@struct.each do |col|
			@types[col[1]] = col[0]
		end

		@buf = ERB.new(template).result(binding)
	end
end

class TdbStructHeaderTask < TdbGenTask
	def initialize(work, name)
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
		super(work, name, template, "#{name}.struct.h")
	end
end

class TdbFormatHeaderTask < TdbGenTask
	def initialize(work, name)
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
		super(work, name, template, "#{name}.format.h")
	end
end

class TdbExtHeaderTask < TdbGenTask
	def initialize(work, name)
		template = %q{
#ifndef <%=upName%>_H
#define <%=upName%>_H

#include "ConstMap.h"
#include "dllHelpers.h"
#include "<%=name%>.struct.h"

class <%=@structName%>s : public ConstMap<int, <%=@structName%>> {
public:
	void load() VISIBLE;
<% if(@index) then @index.each do |args| %>
	void* find<%args.each do |arg|%><%=capitalize(arg)%><%end%>(<%args.each_with_index do |arg, i|%>
		<%if(i!=0)%>,<%end%><%=@types[arg]%> <%=arg%><%end%>
		);
<%end; end%>
};

extern <%=@structName%>s g<%=@structName%>s VISIBLE;

#endif	//<%=upName%>_H
}
		super(work, name, template, "#{name}.h")
	end
end

class TdbCppTask < TdbGenTask
	def initialize(work, name)
		template = %q{
#include "<%=name%>.h"
#include "tdb.h"
#include "<%=name%>.format.h"

static bool sLoaded = false;

void <%=@structName%>s::load() {
	if(sLoaded)
		return;
	sLoaded = true;
	TDB<<%=@structName%>>::fetchTable("<%=@mysql_db_name%>", s<%=@structName%>Formats,
		sizeof(s<%=@structName%>Formats) / sizeof(ColumnFormat), (super&)*this);
}

<%=@structName%>s g<%=@structName%>s;
}
		super(work, name, template, "#{name}.cpp")
	end
end
