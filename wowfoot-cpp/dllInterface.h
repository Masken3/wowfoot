#ifndef DLL_INTERFACE_H
#define DLL_INTERFACE_H

#include <inttypes.h>
#include "dllHelpers.h"

class RequestHandler;

// return non-zero to continue receiving arguments, or zero to stop enumerating.
typedef int (*ArgumentCallback)(void* user, const char* key, const char* value);
typedef void (*GetArguments)(void* src, ArgumentCallback, void* user);

struct ResponseData {
	// Core fills, DLL ignores.
	RequestHandler* handler;
	GetArguments getArgs;
	void* getArgsSrc;
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
