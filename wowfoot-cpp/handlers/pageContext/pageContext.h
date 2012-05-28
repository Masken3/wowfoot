#ifndef PAGE_CONTEXT_H
#define PAGE_CONTEXT_H

#include "chtmlBase.h"

class VISIBLE PageContext {
public:
	PageContext(const char* pt);
	virtual void getResponse2(const char* urlPart, DllResponseData* drd, std::ostream&) = 0;
	virtual void title(std::ostream&) = 0;
	virtual int runPage(std::ostream&) = 0;

	int run(std::ostream&);

	// default implementation throws an exception.
	virtual void httpArgument(const char* key, const char* value) __attribute((noreturn)) VISIBLE;
protected:
	// calls httpArgument.
	void getArguments(ResponseData* rd) VISIBLE;
private:
	int runHead(std::ostream&);
	int runFoot(std::ostream&);
	const char* const pageTitle;
};

void getResponse(const char* urlPart, DllResponseData* drd, PageContext& context) VISIBLE;

#endif	//PAGE_CONTEXT_H
