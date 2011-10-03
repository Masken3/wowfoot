#include "chtmlUtil.h"
#include <stdio.h>
#include <stdexcept>
#include <string.h>
#include <string>

int toInt(const char* s) {
	int i, n;
	int res = sscanf(s, "%i%n", &i, &n);
	if(res != 1 || n != (int)strlen(s)) {
		throw logic_error("toInt("+std::string(s)+")");
	}
	return i;
}

string toupper(string s) {
	for(size_t i=0; i<s.size(); i++) {
		s[i] = toupper(s[i]);
	}
	return s;
}
