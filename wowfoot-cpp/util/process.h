#ifndef UTIL_LINUX_PROCESS_H
#define UTIL_LINUX_PROCESS_H

#include <stdio.h>

#ifdef _MSC_VER
typedef int pid_t;
#endif

class Process {
public:
	Process(bool killOnDestruct);
	~Process();

	void start(const char* filename, const char* argv[], bool haveGetline);
	void vprintf(const char* fmt, va_list args);
	void printf(const char* fmt, ...) __attribute__((format (printf, 2, 3)));
	void getline(char* buf, size_t bufSize);
	int get_stdout(void) { return mFdin; }
	bool isOpen() const { return mFout != NULL; }

	void kill(void);
private:
	FILE* mFout;
	FILE* mFin;
	int mFdin;
	pid_t mPid;
	const bool mKoD;
};

#endif	//UTIL_LINUX_PROCESS_H
