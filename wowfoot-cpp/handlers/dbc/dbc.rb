DbcColumn = Struct.new(:type, :name, :offset)
Point2D = Struct.new(:x, :y)
AS = Struct.new(:name, :members, :count)
ARRAY = Struct.new(:type, :offset, :count)

# member.
def m(a,b,c)
	DbcColumn.new(a,b,c)
end
# float, 2-dimensional.
def f2(name, x, y)
	DbcColumn.new(:f2, name, Point2D.new(x, y))
end
# array of structs
def as(structName, arrayName, members, count)
	DbcColumn.new(:as, arrayName, AS.new(structName, members, count))
end
def ar(type, name, offset, count)
	DbcColumn.new(:ar, name, ARRAY.new(type, offset, count))
end

class DbcCppTask < MemoryGeneratedFileTask
	def initialize(work, name)
		dir = "build/#{name}"
		@NAME = "#{dir}/#{name}.cpp"
		super(work, @NAME)
		src = "handlers/#{name}/#{name}.rb"
		instance_eval(open(src).read, src)
		@prerequisites << DirTask.new(work, dir)
		@prerequisites << DbcHeaderTask.new(work, name)

		template = %q(
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbcfile.h"
#include "dbc.h"
#include "<%=name%>.h"
#include "util/criticalSection.h"

static CriticalSectionLoadGuard sCS;
<%=@plural%> g<%=@plural%>;

// we need to keep this object alive, so the string table remains valid.
static DBCFile sDbc("DBFilesClient\\\\<%=@dbcName%>.dbc");

void <%=@plural%>::load() {
	LOCK_AND_LOAD;
	DBC::load();

	printf("Opening <%=@dbcName%>.dbc...\n");
	bool res = sDbc.open();
	assert(res);
	printf("Extracting %" PRIuPTR " <%=@plural.downcase%>...\n", sDbc.getRecordCount());
	for(DBCFile::Iterator itr = sDbc.begin(); itr != sDbc.end(); ++itr) {
		<%=@preRow%>
		const DBCFile::Record& r(*itr);
		int id = r.getInt(<%=@id%>);
		<%=@singular%> s;<% @struct.each do |col| if(col.type == :f2) %>
		s.<%=col.name%>.x = r.getFloat(<%=col.offset.x%>);
		s.<%=col.name%>.y = r.getFloat(<%=col.offset.y%>);<%elsif(col.type == :as) then s = col.offset%>
		for(int i=0; i<<%=s.count%>; i++) {<% s.members.each do |m| %>
			s.<%=col.name%>[i].<%=m.name%> = r.get<%=m.type.to_s.capitalize%>(<%=m.offset%> + i);<%end%>
		}<% elsif(col.type == :ar) then a = col.offset %>
		for(int i=0; i<<%=a.count%>; i++) {
			s.<%=col.name%>[i] = r.get<%=a.type.to_s.capitalize%>(<%=a.offset%> + i);
		}<%else%>
		s.<%=col.name%> = r.get<%=col.type.to_s.capitalize%>(<%=col.offset%>);<%end;end%>
		<%=@midRow%>
		insert(pair<int, <%=@singular%>>(id, s));
		<%=@postRow%>
	}
}
)
		@buf = ERB.new(template).result(binding)
	end
end

class DbcHeaderTask < MemoryGeneratedFileTask
	CType = {
		:int => 'int',
		:string => 'const char*',
		:float => 'float',
		:f2 => 'Coord2D',
	}
	def initialize(work, name)
		dir = "build/#{name}"
		@NAME = "#{dir}/#{name}.h"
		super(work, @NAME)
		src = "handlers/#{name}/#{name}.rb"
		instance_eval(open(src).read, src)
		@prerequisites << DirTask.new(work, dir)

		hasCoord2D = false
		@struct.each do |col|
			hasCoord2D = true if(col.type == :f2)
		end

		template = %q(
#ifndef <%=@upperCase%>_H
#define <%=@upperCase%>_H

<%if(hasCoord2D)%>#include "coord2d.h"<%end%>
#include "ConstMap.h"
#include "dllHelpers.h"
#include <unordered_map>

using namespace std;

struct <%=@singular%> {<% @struct.each do |col| if(col.type == :as) then s = col.offset %>
	struct <%=s.name%> {<% s.members.each do |m| %>
		<%=CType[m.type]%> <%=m.name%>;<%end%>
	};
	<%=s.name%> <%=col.name%>[<%=s.count%>];<%elsif(col.type == :ar) then a = col.offset %>
	<%=CType[a.type]%> <%=col.name%>[<%=a.count%>];<%else%>
	<%=CType[col.type]%> <%=col.name%>;<%end;end%>
};

class <%=@plural%> : public ConstMap<int, <%=@singular%>> {
public:
	<%=@plural%>() : name("<%=@singular.downcase%>") {}
	void load() VISIBLE;

	const char* const name;
};

extern <%=@plural%> g<%=@plural%> VISIBLE;

#endif	//<%=@upperCase%>_H

)
		@buf = ERB.new(template).result(binding)
	end
end
