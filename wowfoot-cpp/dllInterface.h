#ifndef DLL_INTERFACE_H
#define DLL_INTERFACE_H

#include <inttypes.h>

class IdHandler;
class RequestHandler;

struct ResponseData {
	// Core fills, DLL ignores.
	RequestHandler* handler;
};

struct DllResponseData : ResponseData {
	// DLL fills
	int code;	// HTTP response code
	void* text;
	uint64_t size;	// size of text
	void* user;
};

typedef void (*DllGetResponse)(const char* urlPart, DllResponseData*);

// must not attempt to delete[] or free() \a drd.
typedef void (*DllCleanup)(DllResponseData* drd);

#endif	//DLL_INTERFACE_H
