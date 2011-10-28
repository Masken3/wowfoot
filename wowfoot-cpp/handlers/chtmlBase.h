#ifndef CHTML_BASE_H
#define CHTML_BASE_H

#include "dllInterface.h"
#include <ostream>

class ChtmlContext {
public:
	virtual int run(std::ostream&) = 0;
};

class PageContext {
public:
	virtual void getResponse2(const char* urlPart, DllResponseData* drd, std::ostream&) = 0;
};

void getResponse(const char* urlPart, DllResponseData* drd, PageContext& context) VISIBLE;

#endif	//CHTML_BASE_H
