#include "chunkedFile.h"
#include <assert.h>
#include <stdio.h>

ChunkedFile::ChunkedFile(const char* start, uint32_t size)
: mStart(start), mPtr(start), mEnd(start + size)
{
}

Chunk ChunkedFile::getChunk() {
	if(mPtr < mEnd) {
		Chunk c = *(Chunk*)mPtr;
		if(c.size == 0) {
			c.data = 0;
			mPtr += c.size + 8;
			return c;
		}
		c.data = mPtr + 8;
		if(c.data >= mEnd) {
			printf("%i < %i\n", (mPtr-mStart), (mEnd-mStart));
		}
		assert(c.data < mEnd);
		mPtr += c.size + 8;
		//assert(c.size > 0);
		return c;
	} else {
		Chunk c = {{0},0,0};
		return c;
	}
}
