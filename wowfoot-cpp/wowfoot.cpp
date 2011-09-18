/* Contains main() and attendant code.
*/

#include "config.h"
#include <set>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <microhttpd.h>

using namespace std;

static void prepareHttpd(int port);
static void runHttpd() __attribute__((noreturn));
static int requestHandler(void *cls, MHD_Connection *connection, const char *url,
	const char *method, const char *version,
	const char *upload_data, size_t *upload_data_size, void **con_cls);
static void mountIdPage(const char* name);
static void mountTextIdPage(const char* name);

int main() {
	prepareHttpd(CONFIG_PORT);

	mountIdPage("npc");
	mountIdPage("quest");
	mountIdPage("item");
	mountIdPage("zone");
	mountIdPage("object");
	mountIdPage("faction");
	mountIdPage("achievement");
	mountTextIdPage("search");

	runHttpd();
}

static MHD_Daemon* sMhd;
static void prepareHttpd(int port) {
	sMhd = MHD_start_daemon(MHD_USE_DEBUG | MHD_USE_THREAD_PER_CONNECTION, port, NULL, NULL, requestHandler, NULL);
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
		int max_fd = 0;
		int res;
		unsigned long long timeout;
		struct timeval tv, *tvp;

		/*res = MHD_run(sMhd);
		printf("MHD_run: %i\n", res);
		assert(res == MHD_YES);

		res = MHD_get_fdset(sMhd, &rf, &wf, &ef, &max_fd);
		printf("MHD_get_fdset: %i (%i)\n", res, max_fd);
		assert(res == MHD_YES);*/

		res = MHD_get_timeout(sMhd, &timeout);
		printf("MHD_get_timeout: %i (%llu)\n", res, timeout);
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		tvp = (res == MHD_YES) ? &tv : NULL;

		res = select(max_fd, &rf, &wf, &ef, tvp);
		printf("select: %i\n", res);
		assert(res > 0);
	}
}

set<const char*> sIdPages;

static int requestHandler(void *cls, MHD_Connection *connection, const char *url,
	const char *method, const char *version,
	const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	printf("requestHandler %s\n", url);
	int res;
	MHD_Response* resp;
	resp = MHD_create_response_from_data(2, (void*)"yo", 0, 0);
	res = MHD_queue_response(connection, 200, resp);
	assert(res == MHD_YES);
	return MHD_YES;
}

static void mountIdPage(const char* name) {
	sIdPages.insert(name);
	// load dll
}

static void mountTextIdPage(const char* name) {
	sIdPages.insert(name);
	// load dll
}
