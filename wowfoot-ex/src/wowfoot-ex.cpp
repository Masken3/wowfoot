#include "config.h"
#include "libs/mpq_libmpq04.h"
#include "libs/dbcfile.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "libs/blp/MemImage.h"

using namespace std;

struct F2 {
	float x, y;
};

static void extractWorldMap(const char* name);

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
		printf("%i, %i, '%s', %fx%f, %fx%f, %i, %i\n",
			map, at, name, a.x, a.y, b.x, b.y, vmap, dmap);
		
		extractWorldMap(name);
	}

	// Records of WMA where 'at' == 0 are continents.
	// The world map images are found in interface/worldmap.
	// They are numbered 1 to 12 and ordered in a 4x3 grid,
	// left-to-right, top-to-bottom. Tiles are 256x256 pixels,
	// making the full image 1024x768 pixels.
	// I suspect the client would bilinear-scale these textures for higher resolutions.

	// Now: extract them!
#if 0
	MPQFile testBlp("interface\\worldmap\\azeroth\\azeroth1.blp");
	printf("size: %"PRIuPTR"\n", testBlp.getSize());
	MemImage img;
	img.LoadFromBLP((const BYTE*)testBlp.getBuffer(), (DWORD)testBlp.getSize());
	img.SaveToPNG("azeroth1.png");
#endif
	return 0;
}

static const unsigned int TILE_WIDTH = 256;
static const unsigned int TILE_HEIGHT = 256;

static void extractWorldMap(const char* name) {
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
		//printf("%s%i.blp: alpha: %i. palette: %i\n", name, i+1, img.HasAlpha(), img.IsPalettized());
	}
	printf("BLPs loaded.\n");
}
