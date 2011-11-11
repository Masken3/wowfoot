#include "util/exception.h"
#include <string.h>
#include <stdio.h>

void throwERRNO() {
	int err = errno;
	char buf[2048];
	perror("perror");
	sprintf(buf, "Errno %i (%s)", err, strerror(err));
	throw Exception(buf);
}

void Exception::stream(std::ostream& o) const {
	o << "Exception: "<<what()<<"\n";
	o << "Stack trace:\n"<<stack_trace()<<"\n";
}
