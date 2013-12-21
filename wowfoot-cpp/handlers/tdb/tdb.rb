# if name, then it's a single.
# otherwise it's an array.
# array may be nil or :key if single.
# :key means it's an SQL primary key column.
# if type == :struct, then dbName is the cName of the struct, array is SqlColumns.
# count is > 1, postfix and cName are ignored.
SqlColumn = Struct.new(:type, :dbName, :array, :count, :postfix, :cName, :parentStruct)
def c(type, dbName, array = nil, count = nil, postfix = '')
	col = SqlColumn.new(type, dbName, array, count, postfix)
	col.cName = dbName
	return col
end
def mc(type, array, count, postfix = '')
	return c(type, nil, array, count, postfix)
end
def renamed(type, dbName, cName, array = nil, count = nil, postfix = '')
	col = c(type, dbName, array, count, postfix)
	col.cName = cName
	return col
end

class ERB
	def setFile(fname)
		@filename = fname
	end
end

class String
	def tdbCapitalize
		return self[0,1].capitalize + self[1..-1]
	end
end

# escape C++ keywords
def cEscape(name)
	return 'class_' if(name.to_s == 'class')
	return name
end

Table = Struct.new(:sql, :structName)

class TdbGenTask < MemoryGeneratedFileTask
	FORMATS = {
		:int => 'CDT_INT',
		:string => 'CDT_STRING',
		:float => 'CDT_FLOAT',
	}
	def load(name, dstName)
		return if(@loaded)
		src = "handlers/#{name}/#{name}.rb"
		@builddir = DirTask.new(TDB_BUILDDIR + '/' + name)
		@prerequisites = [@builddir]
		instance_eval(open(src).read, src)
		@loaded = true
		if(!@tables)
			if(@prefixes)
				@tables = []
				@prefixes.each do |pre|
					@tables << Table.new("#{pre}#{@mysql_db_name}", "#{pre.tdbCapitalize}#{@structName}s")
				end
			else
				@tables = [Table.new(@mysql_db_name, "#{@structName}s")]
			end
		end
	end
	def initialize(name, template, dstName)
		load(name, dstName)
		dst = "#{@builddir}/#{dstName}"
		upName = name.upcase

		@names = {}	# members by name. includes sub-members of arrays.
		@struct.each do |col|
			if(col.cName)
				# if type == :struct, then dbName is the cName of the struct, array is SqlColumns.
				# count is > 1, postfix and cName are ignored.
				if(col.type == :struct)
					raise if(col.count <= 1)
					#puts "struct #{col.cName}"
					col.array.each do |n|
						#puts n.cName.to_s
						n.postfix = ".#{col.cName}"
						n.parentStruct = col
						@names[n.cName] = n
					end
				else
					@names[col.cName] = col
				end
			else
				col.array.each do |n|
					@names[n] = col
				end
			end
		end

		# make sure indexes are valid.
		if(@index)
			@index.each do |i|
				if(i.is_a?(Hash))
					i = i[:keys]
				end
				i.each do |iPart|
					valid = false
					@names.each do |colName, col|
						#puts "#{colName.inspect} - #{iPart.inspect}"
						if(colName == iPart)
							valid = true
							#puts "Found!"
							break
						end
					end
					if(!valid)
						raise "#{name}: index part #{iPart} not found in struct!"
					end
				end
			end
		end

		# todo: count keys. if more than one, and that one is not :int, raise error.

		#p self.class.to_s
		#puts template
		@buf = ERB.new(template).result(binding)
		super(dst)
	end
end

DO_INDEX = %q{ @index.each do |args|
if(args.is_a?(Hash))
	capArgs = args[:name]
	customCpp = args[:customCpp]
	args = args[:keys]
else
	capArgs = args.collect {|arg| arg.to_s.tdbCapitalize; }.join
	customCpp = false
end
if(args.size == 1 && @names[args[0]].type == :int)
	iitr = 'IntItr'
	imap = 'IntMap'
	istruct = 'int'
	ipair = 'IntPair'
	isInt = true
else
	iitr = capArgs + 'Itr'
	imap = capArgs + 'Map'
	istruct = capArgs + 'Struct'
	ipair = capArgs + 'Pair'
	isInt = false
end}
IF_INDEX = '<% if(@index) then' + DO_INDEX + '%>'
IF_DEFINE = %q{<%
if(isInt)
if(!@intMapDefined)
@intMapDefined = true%>
	typedef unordered_multimap<int, const <%=@structName%>*> <%=imap%>;
	typedef <%=imap%>::const_iterator <%=iitr%>;
	typedef pair<<%=iitr%>, <%=iitr%>> <%=ipair%>;<%
end
else%>
	struct <%=istruct%> {<%args.each do |arg|%>
		<%=@names[arg].type%> <%=cEscape(arg)%>;<%end%>
		bool operator==(const <%=istruct%>& o) const {
			return<% args.each_with_index do |arg, i| %>
				<%if(i!=0)%>&& <%end%>this-><%=cEscape(arg)%> == o.<%=cEscape(arg)%><%end%>;
		}
		size_t operator()(const <%=istruct%>& o) const {
			return<% args.each do |arg| %>
				hash<int>()(o.<%=cEscape(arg)%> + <%end%>0
				<% args.each do |arg| %>)<%end%>;
		}
	};
	typedef unordered_multimap<<%=istruct%>, const <%=@structName%>*, <%=istruct%>> <%=imap%>;
	typedef <%=imap%>::const_iterator <%=iitr%>;
	typedef pair<<%=iitr%>, <%=iitr%>> <%=ipair%>;
<%end%>}

class TdbStructHeaderTask < TdbGenTask
	def initialize(name)
		template = %q(
#ifndef <%=upName%>_STRUCT_H
#define <%=upName%>_STRUCT_H

struct <%=@structName%> {<% @struct.each do |col|
	if(col.type == :struct) %>
	struct <%=col.dbName%> {<%col.array.each do |m| %>
		<%=m.type%> <%=cEscape(m.cName)%>;<%end%>
	} <%=col.cName%>[<%=col.count%>];<%
	elsif(col.cName) %>
	<%=col.type%> <%=cEscape(col.cName)%>;<%else
	col.array.each do |name| %>
	<%=col.type%> <%=cEscape(name)%>[<%=col.count%>];<%end; end; end; if(@containerType == :set)%>
	bool operator==(const <%=@structName%>& o) const {
		return<% @struct.each_with_index do |col, i| if(col.array == :key) %>
			<%if(i!=0)%>&& <%end%>this-><%=cEscape(col.cName)%> == o.<%=cEscape(col.cName)%><%end; end%>;
	}
	size_t operator()(const <%=@structName%>& o) const {
		return<% @struct.each do |col| if(col.array == :key) %>
			hash<int>()(o.<%=cEscape(col.cName)%> + <%end; end%>0
			<% @struct.each do |col| if(col.array == :key) %>)<%end; end%>;
	}
<%end%>
<% if(@extraMembers) then @extraMembers.each do |m| %>
	<%=m%><%end;end%>
};

#endif	//<%= upName %>_STRUCT_H
)
		super(name, template, "#{name}.struct.h")
	end
end

class TdbFormatHeaderTask < TdbGenTask
	def initialize(name)
		template = %q{
#ifndef <%=upName%>_FORMAT_H
#define <%=upName%>_FORMAT_H

#include <stddef.h>

static const ColumnFormat s<%=@structName%>Formats[] = {<% @struct.each do |col|
	if(!col.array || col.array == :key) %>
{<%=FORMATS[col.type]%>, "<%=col.dbName%>", offsetof(<%=@structName%>, <%=cEscape(col.cName)%>)},<% else
	(1..col.count).each do |i| col.array.each do |name|
	if(col.type == :struct)
		type = name.type
		dbName = name.dbName
		cName = col.cName
		namePostfix = ".#{name.cName}"
	else
		type = col.type
		dbName = name
		cName = name
		namePostfix = ''
	end
%>
{<%=FORMATS[type]%>, "<%=dbName%><%=col.postfix%><%=i%>", offsetof(<%=@structName%>, <%=cEscape(cName)%>[<%=i-1%>]<%=namePostfix%>)},<%end; end; end; end%>
};

#endif	//<%= upName %>_FORMAT_H
}
		super(name, template, "#{name}.format.h")
	end
end

class TdbExtHeaderTask < TdbGenTask
	def initialize(name)
		dstName = "#{name}.h"
		load(name, dstName)
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

class CriticalSectionLoadGuard;

class <%=@structName%>s : public <%=@cppContainer%> {
private:
	CriticalSectionLoadGuard& mCS;
public:
	const char* const mTableName;
	const char* const name;

	<%=@structName%>s(const char* tableName, CriticalSectionLoadGuard&);
	void load() VISIBLE;
) + IF_INDEX + IF_DEFINE + %q(
private:
	<%=imap%> m<%=capArgs%>Map;
public:
	<%=ipair%> find<%=capArgs%>(<%args.each_with_index do |arg, i|%>
		<%if(i!=0)%>,<%end%><%=@names[arg].type%> <%=cEscape(arg)%><%end%>
		) const VISIBLE;
	const <%=imap%>& get<%=capArgs%>Map() const VISIBLE { return m<%=capArgs%>Map; }
<%end; end%>
<%=@extraClassDefinitionCode%>
};

<% @tables.each do |t| %>
extern <%=@structName%>s g<%=t.structName%> VISIBLE;<% end %>

#endif	//<%=upName%>_H
)
		super(name, template, dstName)
	end
end

class TdbCppTask < TdbGenTask
	def initialize(name)
		template = %q(
#define __STDC_FORMAT_MACROS
#include "<%=name%>.h"
#include "tdb.h"
#include <stdio.h>
#include <inttypes.h>
#include "<%=name%>.format.h"
#include "util/criticalSection.h"

// removed: big problem when using multiple tables; only one will get loaded.
//static CriticalSectionLoadGuard sCS;
<%=@extraHeaderCode%>

<%=@structName%>s::<%=@structName%>s(const char* tableName,
	CriticalSectionLoadGuard& cs)
	: mCS(cs), mTableName(tableName),
name("<%=@structName.downcase%>")
{}

void <%=@structName%>s::load() {
	LOCK_AND_LOAD_EX(mCS);
	TDB<<%=@structName%>>::fetchTable(mTableName, s<%=@structName%>Formats,
		sizeof(s<%=@structName%>Formats) / sizeof(ColumnFormat), (super&)*this);<%
if(@index and @index.size > 0)%>
	for(<%=@structName%>s::citr itr = begin(); itr != end(); ++itr) {
		const <%=@structName%>& _ref(<%if(@containerType == :set)%>*itr<%else%>itr->second<%end%>);<%
) + DO_INDEX + %q(
	col = @names[args[0]];
	isArray = (args.size == 1 && !col.cName)
	struct = col.parentStruct
	if(struct)
		prefix = ".#{struct.cName}[i]"
		count = struct.count
		primary = "#{prefix}.#{cEscape(col.cName)}"
		postfix = ""
	elsif(isArray)
		primary = ".#{cEscape(col.array[0])}[i]"
		count = col.count
		postfix = "[i]"
		prefix = ""
	else
		primary = ".#{cEscape(col.cName)}"
		prefix = ""
		count = nil
		postfix = ""
	end
	if(customCpp)
%><%=customCpp%><%
else
	if(count)%>
		for(int i=0; i<<%=count%>; i++) {<%end%>
		if(_ref<%=primary%> != 0) {<%if(isInt)%>
			int key = <%="_ref#{prefix}.#{cEscape(args[0])}#{postfix}"%>;<% else%>
			<%=istruct%> key = {<%args.each_with_index do |arg, i|%>
				<%="_ref#{prefix}.#{cEscape(arg)}#{postfix}"%>,<%end%>
			};<%end%>
			m<%=capArgs%>Map.insert(pair<<%=istruct%>, const <%=@structName%>*>(key, &_ref));
		}<%if(count)%>}<%end; end
end%>
	}<%
) + DO_INDEX + %q(%>
	printf("Loaded %" PRIuPTR " rows into %s\n", m<%=capArgs%>Map.size(), "m<%=capArgs%>Map");<%end%>
<%end%>
<%=@extraInitCode%>
}

<% @tables.each do |t| %>
static CriticalSectionLoadGuard sCS<%=t.structName%>;
<%=@structName%>s g<%=t.structName%>("<%=t.sql%>", sCS<%=t.structName%>);<% end %>

) + IF_INDEX + %q(
<%=@structName%>s::<%=ipair%> <%=@structName%>s::find<%=capArgs%>(<%args.each_with_index do |arg, i|%>
	<%if(i!=0)%>,<%end%><%=@names[arg].type%> <%=cEscape(arg)%><%end%>
	) const
{<%if(!customCpp)%>
	EASSERT(!m<%=capArgs%>Map.empty());<%end; if(isInt)
	key = cEscape(args[0]) else key = 'key'%>
	<%=istruct%> key = {<%args.each_with_index do |arg, i|%>
		<%=cEscape(arg)%>,<%end%>
	};<%end%>
	return m<%=capArgs%>Map.equal_range(<%=key%>);
}
<%end; end%>
)
		super(name, template, "#{name}.cpp")
	end
end
