#ifndef CHTML_UTIL_H
#define CHTML_UTIL_H

#include "dllHelpers.h"

#include <ostream>
#include <sstream>
#include <string>
using namespace std;

// throws an exception if the string cannot be fully converted.
int toInt(const char*) VISIBLE;

string toupper(string s) VISIBLE;

string htmlEscape(const string& src) VISIBLE;
string jsEscape(const string& src) VISIBLE;

void streamHtmlEscape(ostream&, const string& src) VISIBLE;
void streamHtmlEscape(ostream&, char) VISIBLE;

template<class T> void streamName(ostream& os, const T& map, int id) {
	auto* t = map.find(id);
	if(t) {
		os << t->name;
	} else {
		os << map.name << " " << id;
	}
}

template<class T> string toString(const T& t) {
	ostringstream oss;
	oss << t;
	return oss.str();
}

template<class T> string toStringHex(const T& t) {
	ostringstream oss;
	oss << "0x" << hex << t;
	return oss.str();
}

// float not zero (epsilon)
bool fnz(float f) VISIBLE;

#define VECTOR_EACH(vector, itr) \
for(size_t itr=0; itr<vector.size(); itr++)

#endif	//CHTML_UTIL_H
