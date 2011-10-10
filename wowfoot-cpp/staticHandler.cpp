#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "wowfoot.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "win32.h"

using namespace std;

class StaticDirHandler : public RequestHandler {
public:
	StaticDirHandler(const string& localDir);
	virtual ResponseData* handleRequest(const char* urlPart, MHD_Connection*);
	virtual void cleanup(ResponseData*) __attribute__((noreturn));
private:
	const string mLocalDir;
};

#define HANDLE_ERRNO return handleErrno(size, code, __FILE__, __LINE__)

static char* handleErrno(uint64_t& size, int& code, const char* file, int line) {
	char* buf;
	size = asprintf(&buf, "Internal server error. errno: %i (%s)\n"
		"%s:%i\n",
		errno, strerror(errno),
		file, line);
	code = 500;
	return buf;
}

// returns a malloc'd buffer.
// this buffer will contain the file data,
// or an error message if the file could not be read.
// \a code is the appropriate HTTP status code.
static void* readFileForHTTP(const string& filename, uint64_t& size, int& code) {
	char* buf;
	int fd = open(filename.c_str(), O_RDONLY | O_BINARY);
	if(fd < 0) {
		if(errno == ENOENT) {
			size = asprintf(&buf, "File not found: %s\n", filename.c_str());
			puts(buf);
			code = 404;
		} else {
			HANDLE_ERRNO;
		}
		return buf;
	}
	off_t offset = lseek(fd, 0, SEEK_END);
	if(offset < 0) {
		HANDLE_ERRNO;
	}
	size = offset;
	buf = (char*)malloc(size);
	if(!buf) {
		size = asprintf(&buf, "Internal server error. Could not allocate file buffer of size: %"PRIuPTR, size);
		code = 500;
		return buf;
	}
	offset = lseek(fd, 0, SEEK_SET);
	if(offset < 0) {
		HANDLE_ERRNO;
	}
	uint64_t pos = 0;
	while(pos < size) {
		ssize_t res = read(fd, buf + pos, size - pos);
		if(res <= 0) {
			HANDLE_ERRNO;
		}
		pos += res;
	}
	code = 200;
	return buf;
}

StaticDirHandler::StaticDirHandler(const string& localDir)
: mLocalDir(localDir)
{
	// make sure directory exists
}

ResponseData* StaticDirHandler::handleRequest(const char* urlPart, MHD_Connection* conn) {
	int res;
	MHD_Response* resp;

	// load file
	uint64_t size;
	int code;
	void* text = readFileForHTTP(mLocalDir + urlPart, size, code);

	resp = MHD_create_response_from_data(size, text, 0, 1);
	if(code != 200)
		MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
	res = MHD_queue_response(conn, code, resp);
	assert(res == MHD_YES);
	MHD_destroy_response(resp);
	return NULL;
}
void StaticDirHandler::cleanup(ResponseData*) {
	// this function should never be called.
	abort();
}

void mountStaticDirectory(const char* mountName, const char* localPath) {
	insertPattern(PatternPair("/"+string(mountName)+"/", new StaticDirHandler(localPath)));
}
