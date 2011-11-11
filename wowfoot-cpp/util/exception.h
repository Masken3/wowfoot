#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>
#include <ostream>
#include "dllHelpers.h"

#ifdef WIN32
#include "util/win32/sym_engine/exception2.h"

class Exception : public exception2 {
public:
	Exception(const std::string& msg) : exception2(msg) {}
	void stream(std::ostream&) const VISIBLE;
};

#else
#include "util/unix/stackTrace.h"

class Exception {
public:
	Exception(const std::string& msg) VISIBLE;
	void stream(std::ostream&) const VISIBLE;
private:
	std::string mMessage;
	StackTrace mStackTrace;
};

#endif	//WIN32


#define FAIL(msg) throw Exception(msg)

#define ERRNO(a) { if((a) < 0) { printf("On line %i in file %s:\n", __LINE__, __FILE__); throwERRNO(); } }

void throwERRNO() __attribute__((noreturn));

#endif	//EXCEPTION_H
