#include "config.h"
#include "libs/mpq_libmpq04.h"
#include "libs/dbcfile.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "libs/blp/MemImage.h"
#include "util.h"
#include <unordered_map>

using namespace std;

struct F2 {
	float x, y;
};

struct WorldMapOverlay {
	int zone;
	int area;
	const char* name;
	int w;
	int h;
	int left;
	int top;
};

struct WorldMapArea {
	int id;
	int map;	// key to Map
	int area;	// key to AreaTable
	const char* name;
	vector<WorldMapOverlay> overlays;
};
// key: WorldMapArea::id == WorldMapOverlay::zone
typedef unordered_map<int, WorldMapArea> WmaMap;

static void extractWorldMap(const WorldMapArea&);
static void applyOverlay(MemImage& combine, const WorldMapArea& a,
	const WorldMapOverlay& o);

int main() {
	printf("Opening locale.mpq...\n");
	MPQArchive locale(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/locale-"WOW_LOCALE".MPQ");
	MPQArchive patch(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/patch-"WOW_LOCALE".MPQ");
	MPQArchive patch2(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/patch-"WOW_LOCALE"-2.MPQ");
	MPQArchive patch3(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/patch-"WOW_LOCALE"-3.MPQ");

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
	
	WmaMap wmaMap;

	printf("Opening WorldMapArea.dbc...\n");
	DBCFile wma("DBFilesClient\\WorldMapArea.dbc");
	res = wma.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" map areas...\n", wma.getRecordCount());
	FILE* out = fopen("output/WorldMapArea.rb", "w");
	fprintf(out, "WORLD_MAP_AREA = {\n");
	for(DBCFile::Iterator itr = wma.begin(); itr != wma.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		WorldMapArea a;
		a.id = r.getInt(0);
		a.map = r.getInt(1);
		a.area = r.getInt(2);
		a.name = r.getString(3);
		F2 fa = { r.getFloat(6), r.getFloat(4) };
		F2 fb = { r.getFloat(7), r.getFloat(5) };
#if 1
		if(a.area != 0) {
			fprintf(out, "\t%i => { :map => %i, :name => \"%s\", :a => {:x => %.0f, :y => %.0f},"
				" :b => {:x => %.0f, :y => %.0f} },\n",
				a.area, a.map, a.name, fa.x, fa.y, fb.x, fb.y);
		}
#endif
		if(a.id != 0) {	//top-level zone (Azeroth, Kalimdor, Outland, Northrend)
			assert(wmaMap.find(a.id) == wmaMap.end());
			wmaMap[a.id] = a;
		}
	}
	fprintf(out, "}\n");
#if 0
	MPQFile testBlp("interface\\worldmap\\azeroth\\azeroth12.blp");
	printf("size: %"PRIuPTR"\n", testBlp.getSize());
	MemImage img;
	img.LoadFromBLP((const BYTE*)testBlp.getBuffer(), (DWORD)testBlp.getSize());
	img.SaveToPNG("output/azeroth12.png");
#endif
	
	// looks like we may also need AreaTable.dbc.
#if 1
	printf("Opening AreaTable.dbc...\n");
	DBCFile at("DBFilesClient\\AreaTable.dbc");
	res = at.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" AreaTable entries...\n", at.getRecordCount());
	out = fopen("output/AreaTable.rb", "w");
	fprintf(out, "AREA_TABLE = {\n");
	for(DBCFile::Iterator itr = at.begin(); itr != at.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		int map = r.getInt(1);
		int parentId = r.getInt(2);
		int playerLevel = r.getInt(10);
		const char* name = r.getString(11);
		fprintf(out, "\t%i => { :map => %i, :parent => %i, :level => %i, :name => \"%s\" },\n",
			id, map, parentId, playerLevel, name);
	}
	fprintf(out, "}\n");
#endif

	// now for the overlays.
	printf("Opening WorldMapOverlay.dbc...\n");
	DBCFile wmo("DBFilesClient\\WorldMapOverlay.dbc");
	res = wmo.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" map overlays...\n", wmo.getRecordCount());
	out = fopen("output/WorldMapOverlay.txt", "w");
	for(DBCFile::Iterator itr = wmo.begin(); itr != wmo.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		WorldMapOverlay o;
		o.zone = r.getInt(1);
		o.area = r.getInt(2);
		o.name = r.getString(8);
		// If we don't have a name, we don't have a texture.
		if(o.name[0] == 0)
			continue;
		o.w = r.getInt(9);
		o.h = r.getInt(10);
		o.left = r.getInt(11);
		o.top = r.getInt(12);
		int bbtop = r.getInt(13);
		int bbleft = r.getInt(14);
		int bbb = r.getInt(15);
		int bbright = r.getInt(16);
		fprintf(out, "%i, %i, '%s', %ix%i, %ix%i, [%ix%i,%ix%i]\n",
			o.zone, o.area, o.name, o.w, o.h, o.left, o.top, bbleft, bbtop, bbright, bbb);
		assert(wmaMap.find(o.zone) != wmaMap.end());
		wmaMap[o.zone].overlays.push_back(o);

		// w,h is the size of the combined texture, in pixels.
		// left,top is the coordinates on the area map where this texture should be drawn.
		// texture may be split in 1, 2(2x1), 4(2x2) or 6(3x2) parts.
		// sanity-check that the combined texture is rectangular and that all parts fit.
		// also check that it's as at least big as w,h says.
		// only copy the w,h part.
		// we'll ignore the bounding box for now; I think it's for mouse pointers.
	}

	for(WmaMap::const_iterator itr = wmaMap.begin(); itr != wmaMap.end(); ++itr) {
		extractWorldMap(itr->second);
	}

	return 0;
}

static void checkOverlayDimension(const char* name, int actual, int expected) {
	if(actual < expected) {
		int diff = expected - actual;
		if(diff <= 2) {
			printf("Warning: overlay %s is %i < %i (diff %i)\n", name, actual, expected, diff);
		} else {
			printf("Error: overlay %s is %i < %i (diff %i)\n", name, actual, expected, diff);
			assert(false);
		}
	}
}

static void applyOverlay(MemImage& combine, const WorldMapArea& a,
	const WorldMapOverlay& o)
{
	printf("Loading BLPs for %s/%s...\n", a.name, o.name);
	MemImage src[12];
	int srcCount=0;
	// Gotta calculate how many rows and columns there ought to be,
	// since that info doesn't appear to be stored anywhere.
	// There may be surplus texture pieces; these should be discarded.
	int columns=0, rows=0;
	int totalWidth=0, totalHeight=0;
	for(srcCount=0; srcCount<12; srcCount++) {
		MemImage& img(src[srcCount]);
		char buf[256];
		bool res;
		sprintf(buf, "interface\\worldmap\\%s\\%s%i.blp", a.name, o.name, srcCount+1);
		MPQFile blp(buf);
		if(blp.getSize() <= 32) {	// I guess that any valid texture is at least this big.
			break;
		}
		res = img.LoadFromBLP((const BYTE*)blp.getBuffer(), (DWORD)blp.getSize());
		assert(res);
		//res = img.RemoveAlpha();
		assert(res);
		printf("%s: %ix%i\n", buf, img.GetWidth(), img.GetHeight());
#if 0
		sprintf(buf, "output/%s_%s%i.jpeg", a.name, o.name, srcCount+1);
		img.SaveToJPEG(buf);
#endif
		if(totalWidth < o.w) {
			totalWidth += img.GetWidth();
			columns++;
			assert(columns <= 4);
		}
		if((totalWidth >= o.w) && ((srcCount % columns) == 0 || rows == 0)) {
			totalHeight += img.GetHeight();
			rows++;
			assert(columns * rows <= 12);
		}
		// if we have all we need, stop now.
		if(totalHeight >= o.h && (srcCount+1) == (columns * rows))
			break;
	}
	checkOverlayDimension("width", totalWidth, o.w);
	checkOverlayDimension("height", totalHeight, o.h);
	// check that all parts of each row has the same height.
	// check that all parts of each column has the same width.
	for(int y=0; y<rows; y++) {
		DWORD h = src[y*columns].GetHeight();
		for(int x=0; x<columns; x++) {
			assert(h == src[y*columns+x].GetHeight());
		}
	}
	for(int x=0; x<columns; x++) {
		DWORD w = src[x].GetWidth();
		for(int y=0; y<rows; y++) {
			assert(w == src[y*columns+x].GetWidth());
		}
	}
	// draw
	int py=o.top;
	for(int y=0; y<rows; y++) {
		int px=o.left;
		for(int x=0; x<columns; x++) {
			int i = y*columns+x;
			MemImage& img(src[i]);
			// some images are blank. don't draw those.
			if(img.IsBlank()) {
				printf("Blank part: %s%i.blp\n", o.name, i);
			} else {
				DWORD w = MIN(combine.GetWidth() - px, img.GetWidth());
				DWORD h = MIN(combine.GetHeight() - py, img.GetHeight());
				combine.DrawImage(img, px, py, w, h);
			}
			px += img.GetWidth();
		}
		py += src[y*columns].GetHeight();
	}
}

// Records of WMA where 'at' == 0 are continents.
// The world map images are found in interface/worldmap.
// They are numbered 1 to 12 and ordered in a 4x3 grid,
// left-to-right, top-to-bottom. Tiles are 256x256 pixels,
// making the full image 1024x768 pixels.
// I suspect the client would bilinear-scale these textures for higher resolutions.

// Now: extract them!

static const unsigned int TILE_WIDTH = 256;
static const unsigned int TILE_HEIGHT = 256;

static void extractWorldMap(const WorldMapArea& a) {
	// check if we're done already.
	char outputFileName[256];
	sprintf(outputFileName, "output/%s.jpeg", a.name);
	if(fileExists(outputFileName)) {
		printf("%s already exists, skipping...\n", outputFileName);
		return;
	}

	// load BLPs.
	MemImage src[12];
	bool hasAlpha = false, isPalettized;
	for(int i=0; i<12; i++) {
		MemImage& img(src[i]);
		char buf[256];
		sprintf(buf, "interface\\worldmap\\%s\\%s%i.blp", a.name, a.name, i+1);
		MPQFile testBlp(buf);
		if(testBlp.getSize() <= 256) {	//sanity
			printf("Warning: cannot extract %s\n", buf);
			return;
		}
		bool res = img.LoadFromBLP((const BYTE*)testBlp.getBuffer(),
			(DWORD)testBlp.getSize());
		assert(res);
		assert(img.GetWidth() == TILE_WIDTH);
		assert(img.GetWidth() == TILE_HEIGHT);
		if(!hasAlpha) {
			res = img.RemoveAlpha();
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
			combine.Blit(img, x*TILE_WIDTH, y*TILE_HEIGHT);
		}
	}

	for(size_t i=0; i<a.overlays.size(); i++) {
		applyOverlay(combine, a, a.overlays[i]);
	}

	// save as JPEG.
	// Effective area: 1002 x 668 pixels.
	combine.SaveToJPEG(outputFileName);
}
