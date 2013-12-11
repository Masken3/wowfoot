#include "chunkedFile.h"
#include <assert.h>

ChunkedFile::ChunkedFile(const char* start, uint32_t size)
: mPtr(start), mEnd(start + size)
{
}

Chunk ChunkedFile::getChunk() {
	if(mPtr < mEnd) {
		Chunk c = *(Chunk*)mPtr;
		c.data = mPtr + 8;
		assert(c.data < mEnd);
		mPtr += c.size + 8;
		//assert(c.size > 0);
		return c;
	} else {
		Chunk c = {{0},0,0};
		return c;
	}
}
