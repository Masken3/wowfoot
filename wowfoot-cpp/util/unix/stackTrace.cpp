#include "stackTrace.h"
#include "util/exception.h"
#include "util/process.h"
#include <string.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <unordered_map>
#include <vector>
#include <iostream>

using namespace std;

Exception::Exception(const std::string& m) : mMessage(m) {
	fflush(stdout);
	mStackTrace.stream(cout);
	cout.flush();
}
void Exception::stream(std::ostream& o) const {
	mStackTrace.stream(o);
}

void throwERRNO() {
	int err = errno;
	char errbuf[1024], buf[2048];
	perror("perror");
	errbuf[0] = 0;
	strerror_r(err, errbuf, sizeof(errbuf));
	sprintf(buf, "Errno %i (%s)", err, errbuf);
	throw Exception(buf);
}

StackTrace::StackTrace() {
	mDepth = backtrace(mTrace, TRACE_MAX_DEPTH);
}

class LPSMap : public unordered_map<string, Process*> {
public:
	~LPSMap() {
		for(LPSMap::iterator itr=begin(); itr != end(); itr++)
			delete itr->second;
	}
};

void StackTrace::stream(ostream& o) const {
	o << "Native call stack, "<<mDepth<<" frames:\n";

	//find object for each address
	vector<Dl_info> info(mDepth);
	LPSMap lps;
	for(int i=0; i<mDepth; i++) {
		if(!dladdr(mTrace[i], &info[i]))
			throwERRNO();
		const char* fname = info[i].dli_fname;
		//o << "%p: %s (%p)\n", mTrace[i], fname, info[i].dli_fbase);
		//o << "%p: %s\n", info[i].dli_saddr, info[i].dli_sname);

		if(lps.find(fname) == lps.end()) {
			//o << "new one\n");
			Process* lp = new Process(true);
			lps[fname] = lp;
			const char* argv[] = { "", "-fCe", fname, NULL };
			//printf("addr2line -fCe %s\n", fname);
			lp->start("addr2line", argv, true);
		}
	}

	//find line numbers and log frames
	for(int i=0; i<mDepth; i++) {
		Process* lp = lps[info[i].dli_fname];
		size_t trace = (size_t)mTrace[i];
		if(trace > 0x80000000) {	//isn't exe //hacky magic
			trace -= (size_t)info[i].dli_fbase;
		}
		o << (mDepth - i)<<": "<<mTrace[i]<<" ("<<trace<<")";
		lp->printf("%p\n", (void*)trace);

		char fname[1024];
		lp->getline(fname, sizeof(fname));

		char loc[1024];
		lp->getline(loc, sizeof(loc));

		o << " "<<loc<<": "<<fname<<"\n";
	}

	o << "\n";
}
