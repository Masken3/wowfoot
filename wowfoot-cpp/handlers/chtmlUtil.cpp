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

void streamEscape(Escapist e, ostream& os, const string& src) {
	streamEscape(e, os, src.c_str(), src.size());
}

void streamEscape(Escapist e, ostream& os, const char* src) {
	while(*src) {
		e(os, *src);
		src++;
	}
}

void streamEscape(Escapist e, std::ostream& o, const char* text, size_t len) {
	for(size_t i=0; i<len; i++) {
		e(o, text[i]);
	}
}

void streamHtmlEscape(ostream& os, char c) {
	switch(c) {
	case '&': os << "&amp;"; break;
	case '\"': os << "&quot;"; break;
	case '>': os << "&gt;"; break;
	case '<': os << "&lt;"; break;
	default: os << c;
	}
}

void streamUrlEscape(ostream& os, char c) {
	unsigned char uc = c;
	if(uc > 127) {
		os << "%" << hex << (int)uc;
	} else {
		streamHtmlEscape(os, c);
	}
}

void streamWowFormattedText(ostream& os, const string& src) {
	for(size_t i=0; i<src.size(); i++) {
		if(src[i] == '$' && toupper(src[i+1]) == 'B') {
			os << "<br>\n";
			i++;
			continue;
		}
		streamHtmlEscape(os, src[i]);
	}
}

string jsEscape(const string& src) {
	string dst;
	dst.reserve(src.size()*2);	// ensure no reallocs
	for(size_t i=0; i<src.size(); i++) {
		switch(src[i]) {
		case '\'': dst += "\\\'"; break;
		default: dst += src[i];
		}
	}
	return dst;
}

bool fnz(float f) {
	return fabs(f) > 0.0001;
}
