#include "criticalSection.h"
#include "exception.h"

#ifdef WIN32
//	CRITICAL_SECTION mCS;
CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&mCS);
}
CriticalSection::~CriticalSection() {
}
void CriticalSection::enter() {
	EnterCriticalSection(&mCS);
}
void CriticalSection::leave() {
	LeaveCriticalSection(&mCS);
}
#else
//	pthread_mutex_t mMutex;
CriticalSection::CriticalSection()
: mMutex(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
{
}
CriticalSection::~CriticalSection() {
}
void CriticalSection::enter() {
	ERRNO(pthread_mutex_lock(&mMutex));
}
void CriticalSection::leave() {
	ERRNO(pthread_mutex_unlock(&mMutex));
}
#endif
