#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "wowfoot.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "win32.h"

using namespace std;

class StaticDirHandler : public RequestHandler {
public:
	StaticDirHandler(const string& localDir);
	virtual ResponseData* handleRequest(const char* urlPart, MHD_Connection*);
	virtual void cleanup(ResponseData*) __attribute__((noreturn));
	virtual void unload(const char*, size_t) {}
	virtual void load() {}
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
// may return NULL if reqDate >= fileDate.
static char* readFileForHTTP(const string& filename, uint64_t& size, time_t reqDate, time_t& fileDate, int& code) {
	char* buf;
	// open file
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
	// get date
	{
		struct stat s;
		if(fstat(fd, &s) < 0) {
			HANDLE_ERRNO;
		}
		fileDate = s.st_mtime;
	}
	if(fileDate <= reqDate) {
		code = MHD_HTTP_NOT_MODIFIED;
		return NULL;
	}
	// get size
	off_t offset = lseek(fd, 0, SEEK_END);
	if(offset < 0) {
		HANDLE_ERRNO;
	}
	size = offset;
	// allocate buffer
	buf = (char*)malloc(size);
	if(!buf) {
		size = asprintf(&buf, "Internal server error. Could not allocate file buffer of size: %"PRIu64, size);
		code = 500;
		return buf;
	}
	// read file
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
	// report success
	code = 200;
	return buf;
}

StaticDirHandler::StaticDirHandler(const string& localDir)
: mLocalDir(localDir)
{
	// make sure directory exists
}

// returns a pointer to the end of the processed input buffer.
static const char* parse_date(const char *input, struct tm *tm) {
	const char *cp;

	/* First clear the result structure.  */
	memset (tm, '\0', sizeof (*tm));

	/* Try the ISO format first.  */
	cp = strptime (input, "%F", tm);
	if (cp == NULL) {
		/* Does not match.  Try the US form.  */
		cp = strptime (input, "%D", tm);
	}

	return cp;
}


ResponseData* StaticDirHandler::handleRequest(const char* urlPart, MHD_Connection* conn) {
	int res;
	MHD_Response* resp;
	char* text = NULL;
	int code;
	uint64_t size;
	time_t reqDate = 0;

	// get cache date
	const char* reqDateString = MHD_lookup_connection_value(conn, MHD_HEADER_KIND, MHD_HTTP_HEADER_IF_MODIFIED_SINCE);
	if(reqDateString) {
		struct tm t;
		bool error = false;
		const char* end = parse_date(reqDateString, &t);
		if(*end != 0 || end == reqDateString) {
			error = true;
		} else {
			reqDate = timegm(&t);
			if(reqDate < 0)
				error = true;
		}
		if(error) {
			size = asprintf(&text, "Invalid Date header: %s\n", reqDateString);
			code = 400;
		}
	}

	// load file
	time_t fileDate;
	text = readFileForHTTP(mLocalDir + urlPart, size, reqDate, fileDate, code);
	if(text) {
		resp = MHD_create_response_from_data(size, text, 1, 0);
	} else {
		static const char sTextDate[] = "304 Not Modified\n";
		resp = MHD_create_response_from_data(sizeof(sTextDate)-1, (void*)sTextDate, 0, 0);
	}
	assert(resp);
	if(code == 200) {
#if 0	// MHD_add_response_header fails for no apparent reason.
		// Date gets overwritten.
		char buf[64];
		ctime_r(&fileDate, buf);
		//printf("File date: %s\n", buf);
		res = MHD_del_response_header(resp, MHD_HTTP_HEADER_DATE, buf);
		//printf("MHD_del_response_header('%s'): %i\n", MHD_HTTP_HEADER_DATE, res);
		res = MHD_add_response_header(resp, MHD_HTTP_HEADER_DATE, buf);
#endif
	} else {
		res = MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
		assert(res == MHD_YES);
	}
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
