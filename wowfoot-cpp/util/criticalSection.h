#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class NonCopyable {
public:
	NonCopyable() {}
private:
	NonCopyable(const NonCopyable&);
	NonCopyable& operator=(const NonCopyable&);
};

// spinning; multiple calls to enter() are allowed.
class CriticalSection : NonCopyable {
public:
	CriticalSection();
	~CriticalSection();
	void enter();
	void leave();
private:
#ifdef WIN32
	CRITICAL_SECTION mCS;
#else
	pthread_mutex_t mMutex;
#endif
};

class CriticalSectionLocker : NonCopyable {
private:
	CriticalSection& mCS;
public:
	CriticalSectionLocker(CriticalSection& cs) : mCS(cs) {
		mCS.enter();
	}
	~CriticalSectionLocker() {
		mCS.leave();
	}
};

#define LOCK(cs) CriticalSectionLocker _lock(cs)

#endif	//CRITICAL_SECTION_H
