#include "wowfoot.h"
#include <assert.h>
#include "dll/dll.h"

using namespace std;

class IdHandler : public RequestHandler {
public:
	IdHandler(const char* name);
	virtual ResponseData* handleRequest(const char* urlPart, MHD_Connection*);
	virtual void cleanup(ResponseData*);
private:
	DllGetResponse mDllGetResponse;
	DllCleanup mDllCleanup;
	Dll mDll;
};

IdHandler::IdHandler(const char* name) {
	// load DLL. exit on failure.
	static const string DLLEXT =
#ifdef WIN32
	".dll";
#else
	".so";
#endif
	string dllName = dllDir() + name + DLLEXT;
	bool res = mDll.open(dllName.c_str());
	if(!res) {
		printf("Failed to open %s.\n", dllName.c_str());
		abort();
	}
	mDllGetResponse = (DllGetResponse)mDll.get("getResponse");
	assert(mDllGetResponse);
	mDllCleanup = (DllCleanup)mDll.get("cleanup");
	assert(mDllCleanup);
}
ResponseData* IdHandler::handleRequest(const char* urlPart, MHD_Connection* conn) {
	DllResponseData* rd = new DllResponseData;
	mDllGetResponse(urlPart, rd);
	int res;
	MHD_Response* resp;
	resp = MHD_create_response_from_data(rd->size, rd->text, 0, 0);
	res = MHD_queue_response(conn, rd->code, resp);
	assert(res == MHD_YES);
	MHD_destroy_response(resp);
	return rd;
}
void IdHandler::cleanup(ResponseData* rd) {
	mDllCleanup((DllResponseData*)rd);
	delete rd;
}

void mountIdPage(const char* name) {
	insertPattern(PatternPair(string("/")+string(name)+"=", new IdHandler(name)));
}

void mountTextIdPage(const char* name) {
	mountIdPage(name);
}
