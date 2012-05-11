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

typedef void (*Escapist)(ostream&, char);

void streamEscape(Escapist, ostream&, const string& src) VISIBLE;
void streamEscape(Escapist, ostream&, const char* src) VISIBLE;
void streamEscape(Escapist, ostream&, const char* src, size_t len) VISIBLE;

void streamHtmlEscape(ostream&, char) VISIBLE;
void streamUrlEscape(ostream&, char) VISIBLE;

#define ESCAPE(i) streamEscape(streamHtmlEscape, stream, i)

void streamWowFormattedText(ostream&, const string& src) VISIBLE;

template<class T> void streamName(ostream& os, const T& t) {
	streamEscape(streamHtmlEscape, os, t.name);
}
#ifdef DB_QUEST_STRUCT_H
template<> void streamName<Quest>(ostream& os, const Quest& t) {
	streamEscape(streamHtmlEscape, os, t.title);
}
#endif
#define NAME(i) streamName(stream, i)

template<class T> void streamNameById(ostream& os, const T& map, int id) {
	auto* t = map.find(id);
	if(t) {
		streamName(os, *t);
	} else {
		os << map.name << " " << id;
	}
}

template<class T>
void streamNameLinkById(ostream& o, const T& map, int id) {
	o << "<a href=\""<<map.name<<"="<<id<<"\">";
	streamNameById(o, map, id);
	o << "</a>";
}
#define NAMELINK(map, id) streamNameLinkById(stream, map, id)

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
