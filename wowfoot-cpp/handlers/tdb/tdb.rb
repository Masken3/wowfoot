# if name, then it's a single.
# otherwise it's an array.
# array may be nil or :key if single.
# :key means it's an SQL primary key column.
SqlColumn = Struct.new(:type, :name, :array, :count, :postfix)
def c(type, name, array = nil, count = nil, postfix = '')
	SqlColumn.new(type, name, array, count, postfix)
end
def mc(type, array, count, postfix = '')
	return c(type, nil, array, count, postfix)
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
	def load(work, name, dstName)
		return if(@loaded)
		src = "handlers/#{name}/#{name}.rb"
		@prerequisites = [DirTask.new(self, TDB_BUILDDIR)]
		instance_eval(open(src).read, src)
		@loaded = true
	end
	def initialize(work, name, template, dstName)
		load(work, name, dstName)
		dst = "#{TDB_BUILDDIR}/#{dstName}"
		super(work, dst)
		upName = name.upcase

		@names = {}	# members by name. includes sub-members of arrays.
		@struct.each do |col|
			if(col.name)
				@names[col.name] = col
			else
				col.array.each do |n|
					@names[n] = col
				end
			end
		end

		#p self.class.to_s
		#puts template
		@buf = ERB.new(template).result(binding)
	end
end

IF_INDEX = %q{
<% if(@index) then @index.each do |args|
capArgs = args.collect {|arg| arg.to_s.capitalize; }.join
iitr = capArgs + 'Itr'
imap = capArgs + 'Map'
istruct = capArgs + 'Struct'
ipair = capArgs + 'Pair'
%>
}

class TdbStructHeaderTask < TdbGenTask
	def initialize(work, name)
		template = %q(
#ifndef <%=upName%>_STRUCT_H
#define <%=upName%>_STRUCT_H

struct <%=@structName%> {<% @struct.each do |col|
	if(col.name) %>
	<%=col.type%> <%=cEscape(col.name)%>;<%else
	col.array.each do |name| %>
	<%=col.type%> <%=cEscape(name)%>[<%=col.count%>];<%end; end; end; if(@containerType == :set)%>
	bool operator==(const <%=@structName%>& o) const {
		return<% @struct.each_with_index do |col, i| if(col.array == :key) %>
			<%if(i!=0)%>&& <%end%>this-><%=cEscape(col.name)%> == o.<%=cEscape(col.name)%><%end; end%>;
	}
	size_t operator()(const <%=@structName%>& o) const {
		return<% @struct.each do |col| if(col.array == :key) %>
			hash<int>()(o.<%=cEscape(col.name)%> + <%end; end%>0
			<% @struct.each do |col| if(col.array == :key) %>)<%end; end%>;
	}
<%end%>
};

#endif	//<%= upName %>_STRUCT_H
)
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
	if(!col.array || col.array == :key) %>
{<%=FORMATS[col.type]%>, "<%=col.name%>", offsetof(<%=@structName%>, <%=cEscape(col.name)%>)},<% else
	(1..col.count).each do |i| col.array.each do |name| %>
{<%=FORMATS[col.type]%>, "<%=name%><%=col.postfix%><%=i%>", offsetof(<%=@structName%>, <%=cEscape(name)%>[<%=i-1%>])},<%end; end; end; end%>
};

#endif	//<%= upName %>_FORMAT_H
}
		super(work, name, template, "#{name}.format.h")
	end
end

class TdbExtHeaderTask < TdbGenTask
	def initialize(work, name)
		dstName = "#{name}.h"
		load(work, name, dstName)
		@cppContainerName = 'ConstMap'
		@cppContainer = "ConstMap<int, #{@structName}>"
		if(@containerType == :set)
			@cppContainerName = 'ConstSet'
			@cppContainer = "ConstSet<#{@structName}>"
		end
		template = %q(
#ifndef <%=upName%>_H
#define <%=upName%>_H

#include "<%=@cppContainerName%>.h"
#include "dllHelpers.h"
#include "<%=name%>.struct.h"
#include <unordered_map>

using namespace std;

class <%=@structName%>s : public <%=@cppContainer%> {
public:
	void load() VISIBLE;
) + IF_INDEX + %q(
	struct <%=istruct%> {<%args.each do |arg|%>
		<%=@names[arg].type%> <%=arg%>;<%end%>
		bool operator==(const <%=istruct%>& o) const {
			return<% args.each_with_index do |arg, i| %>
				<%if(i!=0)%>&& <%end%>this-><%=arg%> == o.<%=arg%><%end%>;
		}
		size_t operator()(const <%=istruct%>& o) const {
			return<% args.each do |arg| %>
				hash<int>()(o.<%=arg%> + <%end%>0
				<% args.each do |arg| %>)<%end%>;
		}
	};
	typedef unordered_multimap<<%=istruct%>, const <%=@structName%>*, <%=istruct%>> <%=imap%>;
	typedef <%=imap%>::const_iterator <%=iitr%>;
	typedef pair<<%=iitr%>, <%=iitr%>> <%=ipair%>;
private:
	<%=imap%> m<%=imap%>;
public:
	<%=ipair%> find<%=capArgs%>(<%args.each_with_index do |arg, i|%>
		<%if(i!=0)%>,<%end%><%=@names[arg].type%> <%=arg%><%end%>
		) VISIBLE;
<%end; end%>
};

extern <%=@structName%>s g<%=@structName%>s VISIBLE;

#endif	//<%=upName%>_H
)
		super(work, name, template, dstName)
	end
end

class TdbCppTask < TdbGenTask
	def initialize(work, name)
		template = %q(
#define __STDC_FORMAT_MACROS
#include "<%=name%>.h"
#include "tdb.h"
#include <stdio.h>
#include <inttypes.h>
#include "<%=name%>.format.h"

static bool sLoaded = false;

void <%=@structName%>s::load() {
	if(sLoaded)
		return;
	sLoaded = true;
	TDB<<%=@structName%>>::fetchTable("<%=@mysql_db_name%>", s<%=@structName%>Formats,
		sizeof(s<%=@structName%>Formats) / sizeof(ColumnFormat), (super&)*this);
) + IF_INDEX + %q(
	for(<%=@structName%>s::citr itr = g<%=@structName%>s.begin();
		itr != g<%=@structName%>s.end();
		++itr)
	{
		const <%=@structName%>& _ref(<%if(@containerType == :set)%>*itr<%else%>itr->second<%end%>);<%
		col = @names[args[0]]; isArray = (args.size == 1 && !col.name); if(isArray) %>
		for(int i=0; i<<%=col.count%>; i++) { if(_ref.<%=col.array[0]%>[i] != 0) {<%end%>
		<%=istruct%> key = {<%args.each_with_index do |arg, i|%>
			<%if(i!=0)%>,<%end%>_ref.<%=arg%><%if(isArray)%>[i]<%end;end%>
		};
		m<%=imap%>.insert(pair<<%=istruct%>, const <%=@structName%>*>(key, &_ref));<%
		if(isArray) %>
		}}<%end%>
	}
	printf("Loaded %"PRIuPTR" rows into %s\n", m<%=imap%>.size(), "m<%=imap%>");
<%end; end%>
}

<%=@structName%>s g<%=@structName%>s;

) + IF_INDEX + %q(
<%=@structName%>s::<%=ipair%> <%=@structName%>s::find<%=capArgs%>(<%args.each_with_index do |arg, i|%>
	<%if(i!=0)%>,<%end%><%=@names[arg].type%> <%=arg%><%end%>
	)
{
	<%=istruct%> key = {<%args.each_with_index do |arg, i|%>
		<%if(i!=0)%>,<%end%><%=arg%><%end%>
	};
	return m<%=imap%>.equal_range(key);
}
<%end; end%>
)
		super(work, name, template, "#{name}.cpp")
	end
end
