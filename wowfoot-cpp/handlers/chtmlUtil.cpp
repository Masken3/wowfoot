#include "chtmlUtil.h"
#include "util/exception.h"
#include <stdio.h>
#include <stdexcept>
#include <string.h>
#include <string>
#include <math.h>

int toInt(const char* s) {
	int i, n;
	int res = sscanf(s, "%i%n", &i, &n);
	if(res != 1 || n != (int)strlen(s)) {
		throw Exception("toInt("+std::string(s)+")");
	}
	return i;
}

string toupper(string s) {
	for(size_t i=0; i<s.size(); i++) {
		s[i] = toupper(s[i]);
	}
	return s;
}

string htmlEscape(const string& src) {
	//s.to_s.gsub(/&/, "&amp;").gsub(/\"/, "&quot;").gsub(/>/, "&gt;").gsub(/</, "&lt;")
	string dst;
	dst.reserve(src.size()*6);	// ensure no reallocs
	for(size_t i=0; i<src.size(); i++) {
		switch(src[i]) {
		case '&': dst += "&amp;"; break;
		case '\"': dst += "&quot;"; break;
		case '>': dst += "&gt;"; break;
		case '<': dst += "&lt;"; break;
		default: dst += src[i];
		}
	}
	return dst;
}

bool fnz(float f) {
	return fabs(f) > 0.0001;
}
