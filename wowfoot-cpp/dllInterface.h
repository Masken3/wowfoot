#ifndef DLL_INTERFACE_H
#define DLL_INTERFACE_H

#include <inttypes.h>
#include "dllHelpers.h"

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

extern "C" {
typedef void (*DllGetResponse)(const char* urlPart, DllResponseData*);
void getResponse(const char* urlPart, DllResponseData* drd) VISIBLE;

// must not attempt to delete[] or free() \a drd.
typedef void (*DllCleanup)(DllResponseData* drd);
void cleanup(DllResponseData* drd) VISIBLE;
}

#endif	//DLL_INTERFACE_H
