#include "config.h"
#include "libs/mpq_libmpq04.h"
#include "libs/dbcfile.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "libs/blp/MemImage.h"
#include "util.h"

using namespace std;

struct F2 {
	float x, y;
};

//static void extractWorldMap(const char* name);

int main() {
	printf("Opening locale.mpq...\n");
	MPQArchive locale(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/locale-"WOW_LOCALE".MPQ");

	printf("Opening WorldMapContinent.dbc...\n");
	DBCFile wmc("DBFilesClient\\WorldMapContinent.dbc");
	bool res = wmc.open();
	if(!res) {
		printf("DBC open fail, dumping mpq...\n");
		vector<string> files;
		locale.GetFileListTo(files);
		printf("%"PRIuPTR" files:\n", files.size());
		for(size_t i=0; i<files.size(); i++) {
			printf("%s\n", files[i].c_str());
		}
		return 1;
	}
	printf("Extracting %"PRIuPTR" continents...\n", wmc.getRecordCount());
	for(DBCFile::Iterator itr = wmc.begin(); itr != wmc.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int cid = r.getInt(0);
		int mid = r.getInt(1);
		float x1 = r.getFloat(9);
		float y1 = r.getFloat(10);
		float x2 = r.getFloat(11);
		float y2 = r.getFloat(12);
		printf("%i, %i, %gx%g, %gx%g\n", cid, mid, x1, y1, x2, y2);
	}

	mkdir("output");

	printf("Opening WorldMapArea.dbc...\n");
	DBCFile wma("DBFilesClient\\WorldMapArea.dbc");
	res = wma.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" map areas...\n", wma.getRecordCount());
	for(DBCFile::Iterator itr = wma.begin(); itr != wma.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int map = r.getInt(1);
		int at = r.getInt(2);
		const char* name = r.getString(3);
		F2 a = { r.getFloat(6), r.getFloat(4) };
		F2 b = { r.getFloat(7), r.getFloat(5) };
		int vmap = r.getInt(8);
		int dmap = r.getInt(9);
#if 1
		printf("%i, %i, '%s', %.0fx%.0f, %.0fx%.0f, %i, %i\n",
			map, at, name, a.x, a.y, b.x, b.y, vmap, dmap);
		
		//extractWorldMap(name);
#endif
	}
#if 0
	MPQFile testBlp("interface\\worldmap\\azeroth\\azeroth12.blp");
	printf("size: %"PRIuPTR"\n", testBlp.getSize());
	MemImage img;
	img.LoadFromBLP((const BYTE*)testBlp.getBuffer(), (DWORD)testBlp.getSize());
	img.SaveToPNG("output/azeroth12.png");
#endif

	// now for the overlays.
	printf("Opening WorldMapOverlay.dbc...\n");
	DBCFile wmo("DBFilesClient\\WorldMapOverlay.dbc");
	res = wmo.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" map overlays...\n", wmo.getRecordCount());
	for(DBCFile::Iterator itr = wmo.begin(); itr != wmo.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int zone = r.getInt(1);
		int area = r.getInt(2);
		const char* name = r.getString(8);
		int w = r.getInt(9);
		int h = r.getInt(10);
		int left = r.getInt(11);
		int top = r.getInt(12);
		int bbtop = r.getInt(13);
		int bbleft = r.getInt(14);
		int bbb = r.getInt(15);
		int bbright = r.getInt(16);
		printf("%i, %i, '%s', %ix%i, %ix%i, [%ix%i,%ix%i]\n",
			zone, area, name, w, h, left, top, bbleft, bbtop, bbright, bbb);
		// w,h is the size of the combined texture, in pixels.
		// left,top is the coordinates on the area map where this texture should be drawn.
		// texture may be split in 1, 2(2x1), 4(2x2) or 6(3x2) parts.
		// sanity-check that the combined texture is rectangular and that all parts fit.
		// also check that it's as at least big as w,h says.
		// only copy the w,h part.
		// we'll ignore the bounding box for now; I think it's for mouse pointers.
	}

	return 0;
}
#if 0
// Records of WMA where 'at' == 0 are continents.
// The world map images are found in interface/worldmap.
// They are numbered 1 to 12 and ordered in a 4x3 grid,
// left-to-right, top-to-bottom. Tiles are 256x256 pixels,
// making the full image 1024x768 pixels.
// I suspect the client would bilinear-scale these textures for higher resolutions.

// Now: extract them!

static const unsigned int TILE_WIDTH = 256;
static const unsigned int TILE_HEIGHT = 256;

static void extractWorldMap(const char* name) {
	// check if we're done already.
	char outputFileName[256];
	sprintf(outputFileName, "output/%s.png", name);
	if(fileExists(outputFileName)) {
		printf("%s already exists, skipping...\n", outputFileName);
		return;
	}

	// load BLPs.
	MemImage src[12];
	bool hasAlpha = true, isPalettized;
	for(int i=0; i<12; i++) {
		MemImage& img(src[i]);
		char buf[256];
		sprintf(buf, "interface\\worldmap\\%s\\%s%i.blp", name, name, i+1);
		MPQFile testBlp(buf);
		if(testBlp.getSize() <= 256) {	//sanity
			printf("Warning: cannot extract %s\n", buf);
			return;
		}
		img.LoadFromBLP((const BYTE*)testBlp.getBuffer(), (DWORD)testBlp.getSize());
		assert(img.GetWidth() == TILE_WIDTH);
		assert(img.GetWidth() == TILE_HEIGHT);
		if(hasAlpha) {
			bool res = img.AddAlpha();
			assert(res);
		}
		if(i == 0) {
			//hasAlpha = img.HasAlpha();
			isPalettized = img.IsPalettized();
		} else {
			assert(hasAlpha == img.HasAlpha());
			assert(isPalettized == img.IsPalettized());
		}
		//printf("%s%i.blp: alpha: %i. palette: %i\n",
			//name, i+1, img.HasAlpha(), img.IsPalettized());
	}
	printf("BLPs loaded.\n");

	// Gattai!!
	MemImage combine;
	combine.Init(4*TILE_WIDTH, 3*TILE_HEIGHT, hasAlpha, isPalettized);
	for(int y=0; y<3; y++) {
		for(int x=0; x<4; x++) {
			int i=y*4+x;
			MemImage& img(src[i]);
			unsigned tilePitch = MemImage::CalculateBufferBytes(
				TILE_WIDTH, 1, hasAlpha, isPalettized);
			assert(img.GetBufferBytes() == tilePitch * TILE_HEIGHT);
			for(unsigned j=0; j<TILE_HEIGHT; j++) {
				int dstPos = tilePitch * ((y*TILE_HEIGHT + j)*4 + x);
				int srcPos = tilePitch * j;
				memcpy(combine.GetBuffer() + dstPos, img.GetBuffer() + srcPos, tilePitch);
			}
		}
	}

	// save as png.
	combine.SaveToPNG(outputFileName);
}
#endif
