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

class CriticalSectionLoadGuard : public CriticalSection {
private:
	bool mLoaded;
public:
	CriticalSectionLoadGuard() : mLoaded(false) {}
	bool isLoaded() const { return mLoaded; }
	friend class CriticalSectionLoader;
};

class CriticalSectionLoader : NonCopyable {
private:
	CriticalSectionLoadGuard& mCS;
public:
	CriticalSectionLoader(CriticalSectionLoadGuard& cs) : mCS(cs) {
		mCS.enter();
	}
	~CriticalSectionLoader() {
		mCS.mLoaded = true;
		mCS.leave();
	}
};

#define LOCK(cs) CriticalSectionLocker _lock(cs)

#define LOCK_AND_LOAD CriticalSectionLoader _load(sCS); if(sCS.isLoaded()) return

#endif	//CRITICAL_SECTION_H
