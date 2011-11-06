#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#else
#include <sys/wait.h>
#endif

#ifdef _MSC_VER
#define P_NOWAIT _P_NOWAIT
#include <process.h>
#else
#include <unistd.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>

#include "assert.h"
#include "util/exception.h"
#include "process.h"

Process::Process(bool killOnDestruct) : mFout(NULL), mFin(NULL), mPid(-1), mKoD(killOnDestruct) {
}

Process::~Process() {
	if(mFout) {
		ERRNO(fclose(mFout));
		mFout = NULL;
	}
	if(mFin) {
		ERRNO(fclose(mFin));
		mFin = NULL;
	}
	if(mPid > 0) {
		if(mKoD) {
			::printf("Sending killcode to pid %i\n", mPid);
			kill();
		}
		::printf("Waiting on pid %i\n", mPid);
#ifdef WIN32
		//TODO
#else
		int status;
		ERRNO(waitpid(mPid, &status, 0));
		errno = status;
		ERRNO(errno);
		::printf("status: %i\n", status);
#endif
	}
}

void Process::start(const char* filename, const char* argv[], bool haveGetline) {
	int pin[2], pout[2];
#ifdef WIN32
	ERRNO(_pipe(pin, 1024, _O_TEXT));
	ERRNO(_pipe(pout, 1024, _O_TEXT));

	// save old stdio
	int oldStdout = _dup(_fileno(stdout));
	int oldStdin = _dup(_fileno(stdin));

	// Duplicate write end of out-pipe to stdout
	ERRNO(_dup2(pout[1], _fileno(stdout)));
	ERRNO(_close(pout[1]));

	// Duplicate read end of in-pipe to stdin
	ERRNO(_dup2(pin[0], _fileno(stdin)));
	ERRNO(_close(pin[0]));

	// Spawn process
	mPid = _spawnvp(P_NOWAIT, filename, (char* const*)argv);
	if(mPid == -1) {	//process didn't start
		throwERRNO();
	}

	// Restore old stdio
	ERRNO(_dup2(oldStdout, _fileno(stdout)));
	ERRNO(_dup2(oldStdin, _fileno(stdin)));

	// Close duplicates
	ERRNO(_close(oldStdout));
	ERRNO(_close(oldStdin));

#else
	ERRNO(pipe(pin));
	ERRNO(pipe(pout));

	mPid = fork();
	if(mPid == 0) {	//child
		if(dup2(pin[0], STDIN_FILENO) < 0)
			_exit(errno);
		close(pin[1]);
		if(dup2(pout[1], STDOUT_FILENO) < 0)
			_exit(errno);
		close(pout[0]);
		execvp(filename, (char* const*)argv);	//if this function returns, it has failed.
		_exit(errno);
	} else {	//parent
		if(mPid < 0) {	//fork failed
			throwERRNO();
		}
	}
#endif
	::printf("Started '%s' with pid %i\n", filename, mPid);

	mFout = fdopen(pin[1], "w");
	if(!mFout) throwERRNO();
	if(haveGetline) {
		mFin = fdopen(pout[0], "r");
		if(!mFin) throwERRNO();
	} else {
		mFdin = pout[0];
	}
}

void Process::printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	this->vprintf(fmt, args);
	va_end(args);
}

void Process::vprintf(const char* fmt, va_list args) {
	va_list args2;
	va_copy(args2, args);
	ERRNO(::vfprintf(mFout, fmt, args));
	::vprintf(fmt, args2);
	fflush(mFout);
}

void Process::kill() {
#ifdef WIN32
	//TODO
#else
	ERRNO(::kill(mPid, SIGTERM));
#endif
}

#ifdef WIN32
static int getline(char** _buf, size_t* _bufSize, FILE* f) {
	char* buf(*_buf);
	size_t bufSize(*_bufSize);
	_ASSERT(bufSize > 0);
	size_t i=0;
	while(i < bufSize-1) {
		int c = fgetc(f);
		if(c < 0) {
			ERRNO(c);
		}
		if(c == '\n')
			break;
		buf[i] = (char)c;
		i++;
	}
	if(i == bufSize) {
		FAIL("getline buffer overrun");
	}
	::printf("getline returns %i\n", i);
	return (int)i;
}
#endif

void Process::getline(char* buf, size_t bufSize) {
	//LOG("Getting line...\n");
	assert(mFin);
	int res = ::getline(&buf, &bufSize, mFin);
	//LOG("getline: %i\n", res);
	ERRNO(res);
	buf[res - 1] = 0;
}
