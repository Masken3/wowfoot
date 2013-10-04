require 'erb'

DbcColumn = Struct.new(:type, :name, :offset)
Point2D = Struct.new(:x, :y)
AS = Struct.new(:name, :members, :count)
ARRAY = Struct.new(:type, :offset, :count)

if(CONFIG_WOW_VERSION < 20000)
	STRING_LENGTH = 8
else
	STRING_LENGTH = 16
end

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
# array
def ar(type, name, offset, count)
	DbcColumn.new(:ar, name, ARRAY.new(type, offset, count))
end

def luaType(type)
	case(type)
	when :int; return 'number';
	when :float; return 'number';
	when :string; return 'string';
	else; raise "Unknown luaType #{type.inspect}"
	end
end

class DbcCppTask < MemoryGeneratedFileTask
	def initialize(name, options={})
		dir = "build/#{name}"
		@NAME = "#{dir}/#{name}.cpp"
		src = "#{File.dirname(__FILE__)}/../#{name}/#{name}.rb"
		instance_eval(open(src).read, src)
		@prerequisites ||= []
		@prerequisites << DirTask.new(dir)
		@prerequisites << DbcHeaderTask.new(name, options)
		@options = options
		@options[:criticalSection] = true if(options[:criticalSection] != false)

		template = %q(
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbcfile.h"
#include "<%=name%>.h"

<%if(@options[:criticalSection])%>
#include "dbc.h"
#include "util/criticalSection.h"
static CriticalSectionLoadGuard sCS;
<%else%>
#include "../wowfoot-ex/src/dbcList.h"
<%end%>

<%if(@options[:lua])%>
extern "C" {
#include <lauxlib.h>
}

void luaPush<%=@singular%>(lua_State* L, const <%=@singular%>& s) {
	lua_createtable(L, 0, <%=@struct.size%>);
<%@struct.each do |col|%>
	lua_pushstring(L, "<%=col.name%>");
<%if(col.type == :ar); a = col.offset%>
	lua_createtable(L, <%=a.count%>, 0);
	for(int i=0; i<<%=a.count%>; i++) {
		lua_push<%=luaType(a.type)%>(L, s.<%=col.name%>[i]);
		lua_rawseti(L, -2, i+1);
	}
<%elsif(col.type == :as); s = col.offset%>
	lua_createtable(L, <%=s.count%>, 0);
	for(int i=0; i<<%=s.count%>; i++) {
		lua_createtable(L, 0, <%=s.members.size%>);
		<% s.members.each do |m| %>
		lua_pushstring(L, "<%=m.name%>");
		lua_push<%=luaType(m.type)%>(L, s.<%=col.name%>[i].<%=m.name%>);
		lua_settable(L, -3);
		<% end %>
		lua_rawseti(L, -2, i+1);
	}
<%else%>
	lua_push<%=luaType(col.type)%>(L, s.<%=col.name%>);
<%end%>
	lua_settable(L, -3);
<%end%>
}
<%end%>

<%=@plural%> g<%=@plural%>;

// we need to keep this object alive, so the string table remains valid.
static DBCFile sDbc("DBFilesClient\\\\<%=@dbcName%>.dbc");

void <%=@plural%>::load() {
<%if(@options[:criticalSection])%>
	LOCK_AND_LOAD;
	DBC::load();
<%else%>
	loadMpqFiles();
<%end%>

	printf("Opening <%=@dbcName%>.dbc...\n");
	bool res = sDbc.open();
	assert(res);
	printf("Extracting %" PRIuPTR " <%=@plural%>...\n", sDbc.getRecordCount());
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
		super(@NAME)
	end
end

class DbcHeaderTask < MemoryGeneratedFileTask
	CType = {
		:int => 'int',
		:string => 'const char*',
		:float => 'float',
		:f2 => 'Coord2D',
	}
	def initialize(name, options={})
		dir = "build/#{name}"
		@NAME = "#{dir}/#{name}.h"
		src = "#{File.dirname(__FILE__)}/../#{name}/#{name}.rb"
		instance_eval(open(src).read, src)
		@prerequisites ||= []
		@prerequisites << DirTask.new(dir)
		@options = options

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

<%if(@options[:lua])%>
extern "C" {
#include <lua.h>
}
void luaPush<%=@singular%>(lua_State* L, const <%=@singular%>&) VISIBLE;
<%end%>

extern <%=@plural%> g<%=@plural%> VISIBLE;

<%=@endOfHeader%>
#endif	//<%=@upperCase%>_H

)
		@buf = ERB.new(template).result(binding)
		super(@NAME)
	end
end
