#ifndef STACK_TRACE_H
#define STACK_TRACE_H

#include <ostream>

class StackTrace {
public:
	StackTrace();
	void stream(std::ostream&) const;
private:
	enum { TRACE_MAX_DEPTH=50 };
	void* mTrace[TRACE_MAX_DEPTH];
	int mDepth;
};

#endif	//STACK_TRACE_H
