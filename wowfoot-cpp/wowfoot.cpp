/* Contains main() and attendant code.
*/

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "dllInterface.h"

#include "config.h"
#include <string>
#include "wowfoot.h"

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

	prepareHttpd(CONFIG_PORT);

	mountIdPage("zone");
	mountTextIdPage("search");
	mountIdPage("item");
#if 0
	mountIdPage("npc");
	mountIdPage("quest");
	mountIdPage("object");
	mountIdPage("faction");
	mountIdPage("achievement");
#endif
	mountStaticDirectory("output", "../wowfoot-ex/output/");
	mountStaticDirectory("static", "../wowfoot-webrick/htdocs/static/");

	runHttpd();
}

static MHD_Daemon* sMhd;
static void prepareHttpd(int port) {
	sMhd = MHD_start_daemon(MHD_USE_DEBUG | MHD_USE_THREAD_PER_CONNECTION,
		port, NULL, NULL, requestHandler, NULL,
		MHD_OPTION_NOTIFY_COMPLETED, requestCompletedCallback, NULL, MHD_OPTION_END);
	printf("%p\n", requestCompletedCallback);
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

static PatternMap sRootPatterns;

void insertPattern(PatternPair p) {
	sRootPatterns.insert(p);
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
		}
		++itr;
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
