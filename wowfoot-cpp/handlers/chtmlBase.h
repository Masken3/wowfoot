#ifndef CHTML_BASE_H
#define CHTML_BASE_H

#include "dllInterface.h"
#include <ostream>

class ChtmlContext {
public:
	virtual int run(std::ostream& stream) = 0;
};

void getResponse(DllResponseData* drd, ChtmlContext& context) VISIBLE;

#endif	//CHTML_BASE_H
