#define __STDC_FORMAT_MACROS
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>
#include "areaMap.h"
#include "win32.h"
#include "util/exception.h"
#include "util/criticalSection.h"

static CriticalSectionLoadGuard sCS;

// throws Exception on error.
static int readInt(int fd);
static void readBlob(int fd, void* dst, int size);
static bool isEof(int fd);

static int readInt(int fd) {
	int i;
	readBlob(fd, &i, sizeof(int));
	return i;
}

static void readBlob(int fd, void* dst, int size) {
	char* ptr = (char*)dst;
	char* end = ptr + size;
	while(ptr != end) {
		ssize_t res = read(fd, ptr, end - ptr);
		ERRNO(res);
		assert(res <= end - ptr);
		ptr += res;
	}
}

static bool isEof(int fd) {
	off_t cur = lseek(fd, 0, SEEK_CUR);
	ERRNO(cur);
	off_t end = lseek(fd, 0, SEEK_END);
	ERRNO(end);
	ERRNO(lseek(fd, cur, SEEK_SET));
	return cur == end;
}

static const int NGRIDS = MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_GRIDS;
static const int NCELLS = MAX_NUMBER_OF_CELLS*MAX_NUMBER_OF_CELLS;

AreaMap::~AreaMap() {
	for(super::iterator itr = super::begin(); itr != super::end(); ++itr) {
		int** m = itr->second;
		for(int i=0; i<NGRIDS; i++) {
			if(m[i])
				delete m[i];
		}
		delete m;
	}
}

void AreaMap::load() {
	LOCK_AND_LOAD;

	printf("loading AreaMap...\n");
	int fd = open("../wowfoot-ex/output/AreaMap.bin", O_RDONLY | O_BINARY);
	int nMaps = readInt(fd);
	printf("%i maps.\n", nMaps);
	while(!isEof(fd)) {
		//puts "Tell: #{file.tell}"
		int mapId = readInt(fd);
		//printf("Loading map %i\n", mapId);
		//puts "Tell: #{file.tell}"
		//assert(super::operator[](mapId) == NULL);
		ires res = insert(pair<int, int**>(mapId, (int**)NULL));
		assert(res.second);
		int** grids = new int*[NGRIDS];
		res.first->second = grids;
		// read grid validity
		int validGridCount = 0;
		unsigned char valid[NGRIDS / 8];
		readBlob(fd, valid, sizeof(valid));
		for(int i=0; i < NGRIDS;) {
			for(int j=0; j<8; j++, i++) {
				if((valid[i / 8] & (1 << j)) != 0) {
					grids[i] = new int[NCELLS];
					validGridCount++;
				} else {
					grids[i] = NULL;
				}
			}
		}
		//puts "#{validGridCount} valid grids"
		//puts "Tell: #{file.tell}"
		// read cell data
		int gridIndex = 0;
		int loadedGridCount = 0;
		while(true) {
			while(!grids[gridIndex]) {
				gridIndex++;
				if(gridIndex == NGRIDS)
					break;
			}
			if(gridIndex == NGRIDS)
				break;
			readBlob(fd, grids[gridIndex], NCELLS*4);
			loadedGridCount++;
			gridIndex++;
		}
		assert(gridIndex == NGRIDS);
		assert(loadedGridCount == validGridCount);
	}
	// nMaps is not actually valid, because of some WDTs that can't be loaded.
	//assert(map.size == nMaps);
	printf("Loaded %"PRIuPTR" maps\n", size());
}

AreaMap gAreaMap;
