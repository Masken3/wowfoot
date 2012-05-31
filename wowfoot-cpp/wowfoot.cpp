/* Contains main() and attendant code.
*/

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "dllInterface.h"

#include "config.h"
#include <string>
#include "wowfoot.h"
#include "util/minmax.h"

#ifdef WIN32
#define PRIxLL "I64x"
#else
#define PRIxLL "llx"
#endif

using namespace std;

static void prepareHttpd(int port);
static void runHttpd() __attribute__((noreturn));
static int requestHandler(void *cls, MHD_Connection *connection, const char *url,
	const char *method, const char *version,
	const char *upload_data, size_t *upload_data_size, void **con_cls);
static void requestCompletedCallback(void* cls, MHD_Connection*, void** con_cls,
	MHD_RequestTerminationCode);
static void mountUnload();

static string sDllDir;

const string& dllDir() {
	return sDllDir;
}

int main(int argc, const char** argv) {
	if(argc != 2) {
		printf("Usage: wowfoot-cpp <dll directory>\n");
		exit(1);
	}
	sDllDir = argv[1];

	// required for mbtowc() to work properly on GNU systems.
	setlocale(LC_CTYPE, "");

	prepareHttpd(CONFIG_PORT);

	mountIdPage("zone");
	mountTextIdPage("search");
	mountIdPage("item");
	mountIdPage("itemset");
	mountFormPage("items");
	mountIdPage("spell");
	mountFormPage("spells");
	mountIdPage("npc");
	mountIdPage("achievement");
	mountIdPage("object");
	mountIdPage("quest");
	mountFormPage("quests");
	mountIdPage("faction");
	mountIdPage("title");
	mountIdPage("comment");

	mountUnload();
	mountStaticDirectory("output", "../wowfoot-ex/output/");
	mountStaticDirectory("static", "../wowfoot-webrick/htdocs/static/");
	mountStaticDirectory("icon", "build/icon/");
	mountStaticFile("COPYING", "../COPYING");
	mountSource();

	runHttpd();
}

static MHD_Daemon* sMhd;
static void prepareHttpd(int port) {
	sMhd = MHD_start_daemon(MHD_USE_DEBUG | MHD_USE_THREAD_PER_CONNECTION,
		port, NULL, NULL, requestHandler, NULL,
		MHD_OPTION_NOTIFY_COMPLETED, requestCompletedCallback, NULL, MHD_OPTION_END);
	assert(sMhd);
}

static void runHttpd() {
	// read input and discard it
	//while(getchar() != EOF);

	while(true) {
		fd_set rf;
		FD_ZERO(&rf);
		fd_set wf;
		FD_ZERO(&wf);
		fd_set ef;
		FD_ZERO(&ef);
		int res;
		unsigned long long timeout;

#if 0
		res = MHD_run(sMhd);
		printf("MHD_run: %i\n", res);
		assert(res == MHD_YES);

		res = MHD_get_fdset(sMhd, &rf, &wf, &ef, &max_fd);
		printf("MHD_get_fdset: %i (%i)\n", res, max_fd);
		assert(res == MHD_YES);
#endif

		res = MHD_get_timeout(sMhd, &timeout);
		printf("MHD_get_timeout: %i (%"PRIxLL")\n", res, timeout);

#ifdef WIN32
		Sleep(10000000);
#else
		struct timeval tv;
		struct timeval* tvp;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		tvp = (res == MHD_YES) ? &tv : NULL;
		int max_fd = 0;
		res = select(max_fd, &rf, &wf, &ef, tvp);
		printf("select: %i\n", res);
		assert(res > 0);
#endif
	}
}

static PatternMap sRootPatterns, sExactPatterns;

void insertPattern(PatternPair p) {
	sRootPatterns.insert(p);
}
void insertExactPattern(PatternPair p) {
	sExactPatterns.insert(p);
}

static int requestHandler(void* cls, MHD_Connection* conn, const char* url,
	const char* method, const char* version,
	const char* upload_data, size_t* upload_data_size, void** con_cls)
{
	printf("requestHandler %s\n", url);

	// find handler
	RequestHandler* handler = NULL;
	const char* urlPart = NULL;
	PatternMap::const_iterator itr = sRootPatterns.begin();
	while(itr != sRootPatterns.end()) {
		const string& pattern(itr->first);
		if(strncmp(url, pattern.c_str(), pattern.size()) == 0) {
			handler = itr->second;
			urlPart = url + pattern.size();
			break;
		}
		++itr;
	}
	if(handler == NULL) {
		itr = sExactPatterns.find(url);
		if(itr != sExactPatterns.end()) {
			handler = itr->second;
			urlPart = url + itr->first.size();
		}
	}
	if(handler == NULL) {
		// 404
		static char s404[] = "Request handler not found.";
		MHD_Response* resp = MHD_create_response_from_data(sizeof(s404)-1, s404, 0, 0);
		int res = MHD_queue_response(conn, 404, resp);
		assert(res == MHD_YES);
		MHD_destroy_response(resp);
		*con_cls = NULL;
		return MHD_YES;
	}

	ResponseData* rd = handler->handleRequest(urlPart, conn);
	if(rd)
		rd->handler = handler;
	*con_cls = rd;

	return MHD_YES;
}

void requestCompletedCallback(void* cls, MHD_Connection*, void** con_cls,
	MHD_RequestTerminationCode)
{
	ResponseData* rd = (ResponseData*)*con_cls;
	if(rd)
		rd->handler->cleanup(rd);
}


class UnloadHandler : public RequestHandler {
public:
	ResponseData* handleRequest(const char* urlPart, MHD_Connection* conn) {
		int res;
		MHD_Response* resp;

		// parse & unload
		string responseText = "Unloaded:";
		const char* ptr = urlPart;
		while(*ptr) {
			// find next unload request
			const char* end = strchr(ptr, ',');
			size_t len = end ? end - ptr : strlen(ptr);
			const char* colon = (char*)memchr(ptr, ':', len);
			const char* newName = NULL;
			size_t totalLen = len;
			if(colon) {	// new name for dll
				newName = colon + 1;
				len = colon - ptr;
			}
			// search root patterns
			for(PatternMap::const_iterator itr = sRootPatterns.begin();
				itr != sRootPatterns.end(); ++itr)
			{
				const string& pattern(itr->first);
				size_t min = MIN(len, pattern.size() - 2);
				// if request was found in patterns
				if(strncmp(ptr, pattern.c_str() + 1, min) == 0) {
					// prepare to report it
					responseText += ' ';
					responseText.append(ptr, len);
					// perform the unload
					printf("Unloading %.*s...\n", (int)len, ptr);
					itr->second->unload(newName, totalLen - (len+1));
				}
			}
			ptr += totalLen;
			if(*ptr == ',')
				ptr++;
		}
		printf("Unload complete.\n");

		// send the response
		resp = MHD_create_response_from_data(responseText.size(), (void*)responseText.c_str(), 0, 1);
		MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
		res = MHD_queue_response(conn, 200, resp);
		assert(res == MHD_YES);
		MHD_destroy_response(resp);
		return NULL;
	}
	void cleanup(ResponseData*) __attribute__((noreturn)) {
		// this function should never be called.
		abort();
	}
	void unload(const char*, size_t) {}
	void load() {}
};

static void mountUnload() {
	insertPattern(PatternPair("/unload=", new UnloadHandler()));
}

void loadAllHandlers() {
	for(PatternMap::const_iterator itr = sRootPatterns.begin();
		itr != sRootPatterns.end(); ++itr)
	{
		itr->second->load();
	}
}
