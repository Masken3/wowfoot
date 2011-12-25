#include "wowfoot.h"
#include <assert.h>
#include "dll/dll.h"
#include <sys/stat.h>

using namespace std;

class IdHandler : public RequestHandler {
public:
	IdHandler(const char* name);
	virtual ResponseData* handleRequest(const char* urlPart, MHD_Connection*);
	virtual void cleanup(ResponseData*);
	virtual void unload(const char* newName, size_t newNameLen);
	virtual void load();
private:
	DllGetResponse mDllGetResponse;
	DllCleanup mDllCleanup;
	Dll mDll;
	string mDllName;
	time_t mDllTime;

	void reload();
};

static const string DLLEXT =
#ifdef WIN32
	".dll";
#else
	".so";
#endif

static time_t getTime(const char* filename) {
	struct stat s;
	int res = stat(filename, &s);
	assert(res == 0);
	return s.st_mtime;
}

IdHandler::IdHandler(const char* name)
: mDllName(dllDir() + name + DLLEXT)
{
	reload();
}

void IdHandler::reload() {
	// load DLL. exit on failure.
	mDll.close();
	mDllTime = getTime(mDllName.c_str());
	bool res = mDll.open(mDllName.c_str());
	if(!res) {
		printf("Failed to open %s.\n", mDllName.c_str());
		abort();
	}
	printf("Loaded %s\n", mDllName.c_str());
	mDllGetResponse = (DllGetResponse)mDll.get("getResponse");
	assert(mDllGetResponse);
	mDllCleanup = (DllCleanup)mDll.get("cleanup");
	assert(mDllCleanup);
}

void IdHandler::load() {
	// check if DLL has been updated. If so, reload.
	if(getTime(mDllName.c_str()) != mDllTime) {
		printf("Time mismatch. reloading...\n");
		reload();
		loadAllHandlers();
	}
}

ResponseData* IdHandler::handleRequest(const char* urlPart, MHD_Connection* conn) {
	load();
	DllResponseData* rd = new DllResponseData;
	mDllGetResponse(urlPart, rd);
	int res;
	MHD_Response* resp;
	resp = MHD_create_response_from_data(rd->size, rd->text, 0, 0);
	MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
	res = MHD_queue_response(conn, rd->code, resp);
	assert(res == MHD_YES);
	MHD_destroy_response(resp);
	return rd;
}

void IdHandler::cleanup(ResponseData* rd) {
	mDllCleanup((DllResponseData*)rd);
	delete rd;
}

void IdHandler::unload(const char* newName, size_t newNameLen) {
	mDll.close();
	mDllTime = 0;	// in case the build fails
	if(newName) {
		mDllName.assign(newName, newNameLen);
	}
}

void mountIdPage(const char* name) {
	insertPattern(PatternPair(string("/")+string(name)+"=", new IdHandler(name)));
}

void mountFormPage(const char* name) {
	IdHandler* h = new IdHandler(name);
	insertPattern(PatternPair(string("/")+string(name)+"?", h));
	insertExactPattern(PatternPair(string("/")+string(name), h));
}

void mountTextIdPage(const char* name) {
	mountIdPage(name);
}
