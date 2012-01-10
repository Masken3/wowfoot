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
	string mBaseDllName;
	string mLoadedDllName;
	int mReloadCount;
	struct stat mDllStat;

	void reload();
};

static const string DLLEXT =
#ifdef WIN32
	".dll";
#else
	".so";
#endif

IdHandler::IdHandler(const char* name)
: mBaseDllName(dllDir() + name + DLLEXT)
{
	mReloadCount = 0;
	memset(&mDllStat, 0, sizeof(mDllStat));
	reload();
}

void IdHandler::reload() {
	// load DLL. exit on failure.
	mDll.close();

	struct stat s;
	int res = stat(mBaseDllName.c_str(), &s);
	assert(res == 0);

	// see if the file needs to be renamed
	const char* dllName = mBaseDllName.c_str();
	char buf[1024];
	if(s.st_ino != mDllStat.st_ino && mReloadCount > 0) {
		sprintf(buf, "%s.%i", mBaseDllName.c_str(), mReloadCount);
		remove(buf);
		res = link(mBaseDllName.c_str(), buf);
		assert(res == 0);
		dllName = buf;
	}

	res = mDll.open(dllName);
	if(!res) {
		printf("Failed to open %s.\n", dllName);
		abort();
	}
	printf("Loaded %s\n", dllName);
	mDllGetResponse = (DllGetResponse)mDll.get("getResponse");
	assert(mDllGetResponse);
	mDllCleanup = (DllCleanup)mDll.get("cleanup");
	assert(mDllCleanup);
	mDllStat = s;
	mLoadedDllName = dllName;
	mReloadCount++;
}

void IdHandler::load() {
	struct stat s;
	int res = stat(mBaseDllName.c_str(), &s);
	if(res != 0 && mReloadCount != 0)
		return;
	assert(res == 0);
	// check if DLL has been updated. If so, reload.
	if(s.st_mtime != mDllStat.st_mtime) {
		printf("Time mismatch. reloading...\n");
		reload();
		loadAllHandlers();
	}
}

#if 0
static int gcv(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
	printf("%s: %s\n", key, value);
	return MHD_YES;
}
#else
struct GCV {
	ArgumentCallback ac;
	void* user;
};

static int gcv(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
	GCV* g = (GCV*)cls;
	int res = g->ac(g->user, key, value);
	return res ? MHD_YES : MHD_NO;
}
static void getArgs(void* src, ArgumentCallback ac, void* user) {
	GCV g = { ac, user };
	MHD_Connection* conn = (MHD_Connection*)src;
	MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND, gcv, &g);
}
#endif

ResponseData* IdHandler::handleRequest(const char* urlPart, MHD_Connection* conn) {
	load();
	DllResponseData* rd = new DllResponseData;
#if 0
	MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND, gcv, NULL);
#else
	rd->getArgs = getArgs;
	rd->getArgsSrc = conn;
#endif
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
	mDllStat.st_mtime = 0;	// in case the build fails
	if(newName) {
		assert(false);
		//mDllName.assign(newName, newNameLen);
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
