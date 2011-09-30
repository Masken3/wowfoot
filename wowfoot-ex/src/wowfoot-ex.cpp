#include "config.h"
#include "libs/mpq_libmpq04.h"
#include "libs/dbcfile.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "libs/blp/MemImage.h"
#include "util.h"
#include "libs/map/wdt.h"
#include "libs/map/adt.h"
#include <unordered_map>
#include <fcntl.h>
#include <set>

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
static void extractImage(const char* blpName, const char* pngName);

static const char *CONF_mpq_list[]={
	"common.MPQ",
	"common-2.MPQ",
	"lichking.MPQ",
	"expansion.MPQ",
	"patch.MPQ",
	"patch-2.MPQ",
	"patch-3.MPQ",
	"patch-4.MPQ",
	"patch-5.MPQ",
};


#if defined( __GNUC__ )
#define _open   open
#define _close close
#ifndef O_BINARY
#define O_BINARY 0
#endif
#else
#include <io.h>
#endif

#ifdef O_LARGEFILE
#define OPEN_FLAGS  (O_RDONLY | O_BINARY | O_LARGEFILE)
#else
#define OPEN_FLAGS (O_RDONLY | O_BINARY)
#endif

static bool FileExists( const char* FileName )
{
	int fp = _open(FileName, OPEN_FLAGS);
	if(fp != -1)
	{
		_close(fp);
		return true;
	}

	return false;
}

static void LoadCommonMPQFiles()
{
	char filename[512];
	int count = sizeof(CONF_mpq_list)/sizeof(char*);
	for(int i = 0; i < count; ++i)
	{
		sprintf(filename, WOW_INSTALL_DIR"Data/%s", CONF_mpq_list[i]);
		if(FileExists(filename))
			new MPQArchive(filename);
	}
}

struct InsensitiveComparator {
	// return true if a is less than b, that is, if it goes first in order.
	bool operator()(const string& a, const string& b) {
		return strcasecmp(a.c_str(), b.c_str()) < 0;
	}
};
typedef set<string, InsensitiveComparator> InSet;

static InSet sFileSet;
static void insert(const string& s) {
	sFileSet.insert(s);
}
static void dumpArchiveSet() {
	printf("Dumping list of files in combined MPQ set...\n");
	for(ArchiveSet::const_iterator itr = gOpenArchives.begin(); itr != gOpenArchives.end(); ++itr) {
		(*itr)->GetFileListCallback(&::insert);
	}
	printf("%"PFZT" files, excluding duplicates.\n", sFileSet.size());
#if 0
	FILE* out = fopen("output/mpqSet.txt", "w");
	for(InSet::const_iterator itr = sFileSet.begin(); itr != sFileSet.end(); ++itr) {
		fprintf(out, "%s\n", itr->c_str());
	}
	fclose(out);
#endif
}

static string switchDirSep(string dir) {
	for(size_t i=0; i<dir.size(); i++) {
		if(dir[i] == '\\')
			dir[i] = '/';
	}
	return dir;
}

static bool beginsWithCase(const string& what, const string& with) {
	if(with.size() > what.size())
		return false;
	return strcasecmp(what.substr(0, with.size()).c_str(), with.c_str()) == 0;
}

static bool mkdir_p(const string& path) {
	//printf("mkdir_p(%s)\n", path.c_str());
	size_t pos = 0;
	while(pos < path.size()) {
		size_t sep = path.find('/', pos);
		//printf("mkdir(%s)\n", path.substr(0, sep).c_str());
		if(mkdir(path.substr(0, sep).c_str()) < 0 && errno != EEXIST) {
			printf("mkdir(%s) failed: %m\n", path.substr(0, sep).c_str());
			return false;
		}
		pos = sep + 2;
	}
	return true;
}

static void extractImages() {
	static const char* dumpImageDirectories[] = {
		"Interface\\MoneyFrame\\",
		"Interface\\Minimap\\",
	};
	static const int nImageDirs = sizeof(dumpImageDirectories) / sizeof(char*);
	for(int i=0; i<nImageDirs; i++) {
		const char* did = dumpImageDirectories[i];
		InSet::const_iterator itr = sFileSet.lower_bound(did);
		for(; itr != sFileSet.end() && beginsWithCase(*itr, did); ++itr) {
			const char* blpFileName = strrchr(itr->c_str(), '\\') + 1;
			const char* blpEnd = strrchr(blpFileName, '.');
			bool skip = false;
			if(!blpEnd)
				skip = true;
			else if(strcasecmp(blpEnd, ".blp") != 0)
				skip = true;
			if(skip) {
				printf("Skipping %s\n", itr->c_str());
				continue;
			}
			string subDir = itr->substr(strlen(did), blpFileName - (itr->c_str() + strlen(did)));
			string pngName = "output/" + switchDirSep(did + subDir);
			mkdir_p(pngName);
			pngName += string(blpFileName, blpEnd - blpFileName) + ".png";
			extractImage(itr->c_str(), pngName.c_str());
		}
	}
}

static void extractImage(const char* blpName, const char* pngName) {
	printf("extractImage(%s)\n", blpName);

	MemImage img;
	MPQFile blpFile(blpName);
	if(blpFile.getSize() <= 256) {	//sanity
		printf("Warning: cannot extract %s\n", blpName);
		return;
	}
	bool res = img.LoadFromBLP((const BYTE*)blpFile.getBuffer(),
		(DWORD)blpFile.getSize());
	assert(res);

	// save as PNG.
	img.SaveToPNG(pngName);
}

static void writeBlob(FILE* out, void* data, size_t size) {
	int res = fwrite(data, size, 1, out);
	assert(res == 1);
}
static void writeInt(FILE* out, int data) {
	writeBlob(out, &data, sizeof(int));
}

static void dumpAdt(FILE* out, const char* adt_filename) {
	ADT_file adt;
	if (!adt.loadFile(adt_filename, false)) {
		printf("Error loading %s\n", adt_filename);
		exit(1);
	}
	adt_MCIN* mcin = adt.a_grid->getMCIN();
	for(int y=0; y<ADT_CELLS_PER_GRID; y++) {
		for(int x=0; x<ADT_CELLS_PER_GRID; x++) {
			adt_MCNK* mcnk = mcin->getMCNK(x, y);
			writeInt(out, mcnk->areaid);
		}
	}
}

#if 0	// Format of AreaMap.bin:
int numberOfMaps;
struct {
	int mapId;
	// arrays are row ordered, top row (y=0) first.
	bool : 1 gridValid[64*64];	// packed boolean array: 64*64/8=64*8=512 bytes
	// numberOfValidGrids is the number of 1-bits in gridValid[].
	int cells[8*8][numberOfValidGrids];
} map[numberOfMaps];
#endif

static void dumpAreaMap(bool dumpArea) {
	bool res;
	FILE* out = NULL;
	FILE* mapOut;

	//TODO: dump Map.rb, as a companion to AreaMap.rb

	printf("Opening Map.dbc...\n");
	DBCFile mapDbc("DBFilesClient\\Map.dbc");
	res = mapDbc.open();
	if(!res)
		exit(1);
	printf("Extracting %"PRIuPTR" maps...\n", mapDbc.getRecordCount());
	if(dumpArea) {
		out = fopen("output/AreaMap.bin", "wb");
		writeInt(out, mapDbc.getRecordCount());
	}
	mapOut = fopen("output/Map.rb", "w");
	fprintf(mapOut, "MAP = {\n");
	for(DBCFile::Iterator itr = mapDbc.begin(); itr != mapDbc.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int mid = r.getInt(0);
		const char* name = r.getString(1);

		fprintf(mapOut, "\t%i => \"%s\",\n", mid, name);
		if(!dumpArea)
			continue;

		printf("Extract %s (%i)\n", name, mid);
		// Loadup map grid data
		char mpq_map_name[1024];
		sprintf(mpq_map_name, "World\\Maps\\%s\\%s.wdt", name, name);
		WDT_file wdt;
		if (!wdt.loadFile(mpq_map_name, false)) {
			printf("Error loading %s.wdt, skipping...\n", name);
			continue;
		}
		//printf("Offset: %li bytes.\n", ftell(out));
		writeInt(out, mid);
		//printf("Offset: %li bytes.\n", ftell(out));

		int nValid = 0;
		for(int y = 0; y < WDT_MAP_SIZE; ++y) {
			assert(sizeof(uint32_t) == 4);
			uint32_t blobs[WDT_MAP_SIZE / 32];
			assert(WDT_MAP_SIZE % 32 == 0);
			memset(blobs, 0, sizeof(blobs));
			for(int x = 0; x < WDT_MAP_SIZE; ++x) {
				if(wdt.main->adt_list[y][x].exist) {
					blobs[x / 32] |= (1 << (x % 32));
					nValid++;
				}
			}
			writeBlob(out, &blobs, sizeof(blobs));
		}
		printf("%i valid grids\n", nValid);
		//printf("Offset: %li bytes.\n", ftell(out));

		for(int y = 0; y < WDT_MAP_SIZE; ++y) {
			for(int x = 0; x < WDT_MAP_SIZE; ++x) {
				if (!wdt.main->adt_list[y][x].exist) {
					continue;
				}
				char adt_filename[1024];
				sprintf(adt_filename, "World\\Maps\\%s\\%s_%u_%u.adt", name, name, x, y);
				dumpAdt(out, adt_filename);
			}
		}
	}
	if(dumpArea) {
		fclose(out);
	}
	fprintf(mapOut, "}\n");
	fclose(mapOut);
}

static string escapeQuotes(const char* src) {
	string s;
	s.reserve(strlen(src)*2);
	const char* ptr = src;
	while(*ptr) {
		if(*ptr == '"')
			s += '\\';
		s += *ptr;
		ptr++;
	}
	return s;
}

int main() {
	bool res;
	FILE* out;
	FILE* out2;

	printf("Opening MPQ files:\n");
	MPQArchive locale(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/locale-"WOW_LOCALE".MPQ");
	MPQArchive patch(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/patch-"WOW_LOCALE".MPQ");
	MPQArchive patch2(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/patch-"WOW_LOCALE"-2.MPQ");
	MPQArchive patch3(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/patch-"WOW_LOCALE"-3.MPQ");
	LoadCommonMPQFiles();

	mkdir("output");

	//if(rand() == 42)
	{
		dumpArchiveSet();
		extractImages();
	}

	if(fileExists("output/AreaMap.bin")) {
		printf("AreaMap.bin already exists, skipping...\n");
		dumpAreaMap(false);
	} else {
		dumpAreaMap(true);
	}

	mkdir("output");

	printf("Opening ItemExtendedCost.dbc...\n");
	DBCFile iec("DBFilesClient\\ItemExtendedCost.dbc");
	res = iec.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" costs...\n", iec.getRecordCount());
	out = fopen("output/ItemExtendedCost.rb", "w");
	fprintf(out, "IdCount = Struct.new(:id, :count)\n");
	fprintf(out, "ITEM_EXTENDED_COST = {\n");
	for(DBCFile::Iterator itr = iec.begin(); itr != iec.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		fprintf(out, "\t%i => {", id);
		fprintf(out, ":honorPoints => %i, :arenaPoints => %i, :arenaSlot => %i, :arenaRating => %i",
			r.getInt(1), r.getInt(2), r.getInt(3), r.getInt(14));
		fprintf(out, ", :item => [");
		for(int i=0; i<5; i++) {
			int iid = r.getInt(4+i);
			int count = r.getInt(9+i);
			if(iid != 0 || count != 0)
				fprintf(out, "IdCount.new(%i, %i),", iid, count);
			if((iid == 0) != (count == 0)) {
				// this actually happens. the frontend can try to display it.
				//printf("ItemExtendedCost error!\n");
				//exit(1);
			}
		}
		fprintf(out, "]},\n");
	}
	fprintf(out, "}\n");

	printf("Opening TotemCategory.dbc...\n");
	DBCFile totem("DBFilesClient\\TotemCategory.dbc");
	res = totem.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" totems...\n", totem.getRecordCount());
	out = fopen("output/TotemCategory.rb", "w");
	fprintf(out, "TOTEM_CATEGORY = {\n");
	for(DBCFile::Iterator itr = totem.begin(); itr != totem.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		const char* name = r.getString(1);
		fprintf(out, "\t%i => \"%s\",\n",
			id, name);
	}
	fprintf(out, "}\n");

	printf("Opening Spell.dbc...\n");
	DBCFile spell("DBFilesClient\\Spell.dbc");
	res = spell.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" spells...\n", spell.getRecordCount());
	out = fopen("output/Spell.rb", "w");
	fprintf(out, "SPELL = {\n");
	for(DBCFile::Iterator itr = spell.begin(); itr != spell.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		const char* name = r.getString(136);
		fprintf(out, "\t%i => \"%s\",\n",
			id, escapeQuotes(name).c_str());
	}
	fprintf(out, "}\n");

	printf("Opening WorldMapContinent.dbc...\n");
	DBCFile wmc("DBFilesClient\\WorldMapContinent.dbc");
	res = wmc.open();
	if(!res) {
		printf("DBC open fail.\n");
		return 1;
	}
	printf("Extracting %"PRIuPTR" continents...\n", wmc.getRecordCount());
	out = fopen("output/WorldMapContinent.rb", "w");
	fprintf(out, "WORLD_MAP_CONTINENT = {\n");
	for(DBCFile::Iterator itr = wmc.begin(); itr != wmc.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int cid = r.getInt(0);
		int mid = r.getInt(1);
		float x1 = r.getFloat(9);
		float y1 = r.getFloat(10);
		float x2 = r.getFloat(11);
		float y2 = r.getFloat(12);
		printf("%i, %i, %gx%g, %gx%g\n", cid, mid, x1, y1, x2, y2);
		fprintf(out, "\t%i => { :x1 => %g, :y1 => %g, :x2 => %g, :y2 => %g },\n",
			mid, x1, y1, x2, y2);
	}
	fprintf(out, "}\n");

	printf("Opening Achievement.dbc...\n");
	DBCFile ach("DBFilesClient\\Achievement.dbc");
	res = ach.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" achievements...\n", ach.getRecordCount());
	out = fopen("output/Achievement.rb", "w");
	fprintf(out, "ACHIEVEMENT = {\n");
	for(DBCFile::Iterator itr = ach.begin(); itr != ach.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		int faction = r.getInt(1);
		int map = r.getInt(2);
		int previous = r.getInt(3);
		const char* name = r.getString(4);
		const char* desc = r.getString(21);
		fprintf(out, "\t%i => { :faction => %i, :map => %i, :previous => %i,"
			" :name => \"%s\", :description => \"%s\" },\n",
			id, faction, map, previous, escapeQuotes(name).c_str(),
			escapeQuotes(desc).c_str());
	}
	fprintf(out, "}\n");

	printf("Opening Faction.dbc...\n");
	DBCFile fac("DBFilesClient\\Faction.dbc");
	res = fac.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" factions...\n", fac.getRecordCount());
	out = fopen("output/Faction.rb", "w");
	fprintf(out, "FACTION = {\n");
	for(DBCFile::Iterator itr = fac.begin(); itr != fac.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		int parent = r.getInt(18);
		const char* name = r.getString(23);
		fprintf(out, "\t%i => { :parent => %i, :name => \"%s\" },\n",
			id, parent, name);
	}
	fprintf(out, "}\n");

	printf("Opening QuestFactionReward.dbc...\n");
	DBCFile qfr("DBFilesClient\\QuestFactionReward.dbc");
	res = qfr.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" QuestFactionRewards...\n", qfr.getRecordCount());
	out = fopen("output/QuestFactionReward.rb", "w");
	fprintf(out, "QUEST_FACTION_REWARD = {\n");
	for(DBCFile::Iterator itr = qfr.begin(); itr != qfr.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		fprintf(out, "\t%i => [", id);
		for(int i=1; i<11; i++) {
			fprintf(out, "%i,", r.getInt(i));
		}
		fprintf(out, "],\n");
	}
	fprintf(out, "}\n");

	printf("Opening ItemSet.dbc...\n");
	DBCFile is("DBFilesClient\\ItemSet.dbc");
	res = is.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" item sets...\n", is.getRecordCount());
	out = fopen("output/ItemSet.rb", "w");
	fprintf(out, "ITEM_SET = {\n");
	for(DBCFile::Iterator itr = is.begin(); itr != is.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		const char* name = r.getString(1);
		fprintf(out, "\t%i => { :name => \"%s\" },\n",
			id, name);
	}
	fprintf(out, "}\n");

	printf("Opening QuestSort.dbc...\n");
	DBCFile qs("DBFilesClient\\QuestSort.dbc");
	res = qs.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" quest sorts...\n", qs.getRecordCount());
	out = fopen("output/QuestSort.rb", "w");
	fprintf(out, "QUEST_SORT = {\n");
	for(DBCFile::Iterator itr = qs.begin(); itr != qs.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		const char* name = r.getString(1);
		fprintf(out, "\t%i => \"%s\",\n",
			id, name);
	}
	fprintf(out, "}\n");

	WmaMap wmaMap;
	printf("Opening WorldMapArea.dbc...\n");
	DBCFile wma("DBFilesClient\\WorldMapArea.dbc");
	res = wma.open();
	if(!res)
		return 1;
	printf("Extracting %"PRIuPTR" map areas...\n", wma.getRecordCount());
	out = fopen("output/WorldMapArea.rb", "w");
	fprintf(out, "WORLD_MAP_AREA = {\n");
	out2 = fopen("output/WorldMapArea.data.cpp", "w");
	fprintf(out2, "#include \"WorldMapArea.data.h\"\n");
	fprintf(out2, "const WMAi gWMA[] = {\n");
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
			fprintf(out2, "{ %i, { %i, \"%s\", {%.0f, %.0f},{%.0f, %.0f} } },\n",
				a.area, a.map, a.name, fa.x, fa.y, fb.x, fb.y);
		}
#endif
		if(a.id != 0) {	//top-level zone (Azeroth, Kalimdor, Outland, Northrend)
			assert(wmaMap.find(a.id) == wmaMap.end());
			wmaMap[a.id] = a;
		}
	}
	fprintf(out, "}\n");
	fprintf(out2, "};\n");
	fprintf(out2, "const size_t gnWMA = sizeof(gWMA) / sizeof(WMAi);\n");
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
	fprintf(out, "# encoding: utf-8\n");
	fprintf(out, "AREA_TABLE = {\n");
	out2 = fopen("output/AreaTable.data.cpp", "w");
	fprintf(out2, "#include \"AreaTable.data.h\"\n");
	fprintf(out2, "const ATi gAT[] = {\n");
	for(DBCFile::Iterator itr = at.begin(); itr != at.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		int mapId = r.getInt(1);
		int parentId = r.getInt(2);
		int playerLevel = r.getInt(10);
		const char* name = r.getString(11);
		fprintf(out, "\t%i => { :map => %i, :parent => %i, :level => %i, :name => \"%s\" },\n",
			id, mapId, parentId, playerLevel, name);
		fprintf(out2, "{ %i, { %i, %i, %i, \"%s\" } },\n",
			id, mapId, parentId, playerLevel, name);
	}
	fprintf(out, "}\n");
	fprintf(out2, "};\n");
	fprintf(out2, "const size_t gnAT = sizeof(gAT) / sizeof(ATi);\n");
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
	bool hasAlpha = false, isPalettized = false;
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
	combine.SaveToJPEG(outputFileName, 1002, 668);
}
