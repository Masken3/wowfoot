#ifndef EXCEPTION_H
#define EXCEPTION_H

#ifdef WIN32
#include "util/windows/sym_engine/exception2.h"

class Exception : public exception2 {
public:
	Exception(const std::string& msg) : exception2(msg) {}
	void dump() const {
		printf("Exception: %s\n", what());
		printf("Stack trace:\n%s\n", stack_trace());
	}
};

#else

#include <string>
#include "util/unix/stackTrace.h"
#include "dllHelpers.h"

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
