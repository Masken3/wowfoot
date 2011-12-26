#ifndef CHTML_BASE_H
#define CHTML_BASE_H

#include "dllInterface.h"
#include <ostream>

class ChtmlContext {
public:
	virtual int run(std::ostream&) = 0;
};

class VISIBLE PageContext {
public:
	virtual void getResponse2(const char* urlPart, DllResponseData* drd, std::ostream&) = 0;

	// default implementation throws an exception.
	virtual void httpArgument(const char* key, const char* value) __attribute((noreturn)) VISIBLE;
protected:
	// calls httpArgument.
	void getArguments(ResponseData* rd) VISIBLE;
};

void getResponse(const char* urlPart, DllResponseData* drd, PageContext& context) VISIBLE;

#endif	//CHTML_BASE_H
