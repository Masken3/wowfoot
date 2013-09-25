#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "wowfoot.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "win32.h"
#include "util/exception.h"

using namespace std;

class SourceHandler : public RequestHandler {
public:
	virtual ResponseData* handleRequest(const char* urlPart, MHD_Connection*);
	virtual void cleanup(ResponseData*) __attribute__((noreturn));
	virtual void unload(const char*, size_t) {}
	virtual void load() {}
};

static int sh(const char* cmd) {
	printf("%s\n", cmd);
	return system(cmd);
}

ResponseData* SourceHandler::handleRequest(const char* urlPart, MHD_Connection* conn) {
	assert(*urlPart == 0);
	int res;

	// call TAR
	char* cwd = getcwd(NULL, 0);
	ERRNO(chdir("../.."));
	ERRNO(sh("tar -czf t.tgz -X wowfoot/.gitignore -X wowfoot/.git/info/exclude --exclude=.git wowfoot"));
	ERRNO(chdir(cwd));
	free(cwd);

	// send file
	int code;
	uint64_t size;
	time_t fileDate;
	char* text = readFileForHTTP("../../t.tgz", size, 0, fileDate, code);
	EASSERT(text);
	MHD_Response* resp = MHD_create_response_from_data(size, text, 1, 0);
	assert(resp);
	{
		// todo: correct mime type?
		res = MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, "archive/tgz");
		assert(res == MHD_YES);
		// todo: rename file; have browser treat it as an attachment, so it gets saved to disk.
		res = MHD_add_response_header(resp, "Content-Disposition", "attachment; filename=\"source.tgz\"");
		assert(res == MHD_YES);
	}
	res = MHD_queue_response(conn, code, resp);
	assert(res == MHD_YES);
	MHD_destroy_response(resp);
	return NULL;
}

void SourceHandler::cleanup(ResponseData*) {
	// this function should never be called.
	abort();
}

void mountSource() {
	insertExactPattern(PatternPair("/source", new SourceHandler()));
}
