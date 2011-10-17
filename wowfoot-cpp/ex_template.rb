require 'erb'

EX_CPP = %q{
#include "<%=name%>.data.h"
#include "<%=name%>.h"

static bool sLoaded = false;

void <%=plural%>::load() {
	if(sLoaded)
		return;
	for(size_t i=0; i<gn<%=singular%>; i++) {
		insert(pair<int, <%=singular%>>(g<%=singular%>[i].id, g<%=singular%>[i].a));
	}
	sLoaded = true;
}

<%=plural%> g<%=plural%>;
}

EX_H = %q{
#ifndef <%=upperCase%>_H
#define <%=upperCase%>_H

#include "ConstMap.h"
#include "<%=name%>.struct.h"
#include "dllHelpers.h"

class <%=plural%> : public ConstMap<int, <%=singular%>> {
public:
	void load() VISIBLE;
};

extern <%=plural%> g<%=plural%> VISIBLE;

#endif	//<%=upperCase%>
}

EX_DATA_H = %q{
#include "<%=name%>.struct.h"
#include <stdlib.h>

struct <%=singular%>i { int id; <%=singular%> a; };
extern const <%=singular%>i g<%=singular%>[];
extern const size_t gn<%=singular%>;
}

# Generates *.cpp, *.h and *.data.h for an array imported from wowfoot-ex.
class ExTemplateCpp < MemoryGeneratedFileTask
	def initialize(work, name, singular, plural, upperCase)
		super(work, "build/#{name}/#{name}.cpp")
		b = binding
		@buf = ERB.new(EX_CPP).result(b)

		@prerequisites << DirTask.new(work, "build/#{name}")

		h = MemoryGeneratedFileTask.new(work, "build/#{name}/#{name}.h")
		h.instance_eval do
			@buf = ERB.new(EX_H).result(b)
		end
		@prerequisites << h

		data = MemoryGeneratedFileTask.new(work, "build/#{name}/#{name}.data.h")
		data.instance_eval do
			@buf = ERB.new(EX_DATA_H).result(b)
		end
		@prerequisites << data
	end
end
