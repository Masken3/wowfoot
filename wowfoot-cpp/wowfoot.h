#ifndef WOWFOOT_H
#define WOWFOOT_H

#include <string>
#include <unordered_map>

// microhttpd prerequisites
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#endif
#include <microhttpd.h>

#include "dllInterface.h"

void mountIdPage(const char* name);
void mountTextIdPage(const char* name);
void mountFormPage(const char* name);
void mountStaticDirectory(const char* mountName, const char* localPath);
void mountStaticFile(const char* urlPath, const char* localPath);
void mountSource();

char* readFileForHTTP(const char* filename, uint64_t& size, time_t reqDate, time_t& fileDate, int& code);

// calls load() on all handlers.
void loadAllHandlers();

const std::string& dllDir();

// abstract
class RequestHandler {
public:
	// returns new RequestData.
	// RequestData::handler will be filled by caller.
	virtual ResponseData* handleRequest(const char* urlPart, MHD_Connection*) = 0;
	virtual void cleanup(ResponseData*) = 0;

	// Unload any DLLs the Handler may have loaded.
	// Used only on systems that lock its executable files (Windows).
	virtual void unload(const char* newName, size_t newNameLen) = 0;

	virtual void load() = 0;
};

typedef std::unordered_map<std::string, RequestHandler*> PatternMap;
typedef std::pair<std::string, RequestHandler*> PatternPair;

void insertPattern(PatternPair);
void insertExactPattern(PatternPair);

#endif	//WOWFOOT_H
