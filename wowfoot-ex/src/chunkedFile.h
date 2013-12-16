#include <inttypes.h>

struct Chunk {
	const char id[4];
	uint32_t size;
	const char* data;

	operator bool() {
		return data != 0;
	}
};

class ChunkedFile {
private:
	const char* mStart;
	const char* mPtr;
	const char* mEnd;
public:
	ChunkedFile(const char* start, uint32_t size);

	// data and size are 0 if at end of file.
	Chunk getChunk();
};
