#ifndef CHTML_BASE_H
#define CHTML_BASE_H

#include "dllInterface.h"
#include <ostream>

class ChtmlContext {
public:
	virtual int run(std::ostream&) = 0;
	virtual ~ChtmlContext();
};

#endif	//CHTML_BASE_H
