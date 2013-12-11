#include "config.h"
#include "libs/mpq_libmpq04.h"
#include "libs/dbcfile.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "libs/blp/MemImage.h"
#include "util.h"
#include "dbcList.h"
#include "chunkedFile.h"
#include "wmo.h"
#include "libs/map/wdt.h"
#include "libs/map/adt.h"
#include <stdio.h>
#include <unistd.h>

// todo: WDT (World\Maps\OrgrimmarInstance\OrgrimmarInstance.wdt)
// points to: WMO (World\wmo\Dungeon\KL_OrgrimmarLavaDungeon\LavaDungeon.wmo)

struct F3 {
	float x,y,z;

	bool operator <= (const F3& o) const {
		return x <= o.x && y <= o.y && z <= o.z;
	}
	bool operator >= (const F3& o) const {
		return x >= o.x && y >= o.y && z >= o.z;
	}
};

typedef uint32_t hex32;

#define MOHD_members(m)\
	m(uint, nMaterials)\
	m(uint, nGroups)\
	m(uint, nPortals)\
	m(uint, nLights)\
	m(uint, nModels)\
	m(uint, nDoodads)\
	m(uint, nSets)\
	m(hex32, ambientColor)\
	m(uint, areaTableId)\
	m(F3, cornerA)\
	m(F3, cornerB)\
	m(uint, liquidType)\

struct MOHD {
#define DECLARE_MEMBER(type, name) type name;
	MOHD_members(DECLARE_MEMBER);
};

#define MOGI_members(m)\
	m(hex32, flags)\
	m(F3, cornerA)\
	m(F3, cornerB)\
	m(int, nameIndex) /*MOGN*/\

struct MOGI {
	MOGI_members(DECLARE_MEMBER);
};

static void dumpWMO(const char* mpqPath) {
	MPQFile wmo(mpqPath);
	writeFile("output/OrgrimmarLavaDungeon.wmo", wmo.getBuffer(), wmo.getSize());

	ChunkedFile cf(wmo.getBuffer(), wmo.getSize());
	bool hasMVER = false;
	bool hasMOHD = false;
	MOHD* mohd = NULL;
	while(Chunk c = cf.getChunk()) {
		LOG("chunk %c%c%c%c: %i bytes.\n", c.id[3], c.id[2], c.id[1], c.id[0], c.size);
		if(memcmp(c.id, "REVM", 4) == 0) {
			assert(!hasMVER);
			assert(c.size == 4);
			uint32_t version = *(uint32_t*)c.data;
			LOG("MVER: %i\n", version);
			assert(version == 17);	// vanilla
			hasMVER = true;
			continue;
		}
		assert(hasMVER);
		if(memcmp(c.id, "DHOM", 4) == 0) {
			assert(!hasMOHD);
			assert(c.size == sizeof(MOHD));
			hasMOHD = true;
			mohd = (MOHD*)c.data;
			MOHD& a(*mohd);
			LOG("MOHD:\n");
#define format_uint "%u"
#define format_int "%i"
#define format_hex32 "0x%08x"
#define format_F3 "%.2f %.2f %.2f"
#define arg_uint(name) name
#define arg_int(name) name
#define arg_hex32(name) name
#define arg_F3(name) name.x, name.y, name.z
#define LOG_MEMBER(type, name) LOG(" %s:" format_##type "\n", #name, arg_##type(a.name));
			MOHD_members(LOG_MEMBER);
			continue;
		}
		assert(hasMOHD);
		if(memcmp(c.id, "IGOM", 4) == 0) {
			assert(c.size % sizeof(MOGI) == 0);
			const uint count = c.size / sizeof(MOGI);
			const MOGI* mogi = (MOGI*)c.data;
			for(uint i=0; i<count; i++) {
				LOG("MOGI %u:\n", i);
				const MOGI& a(mogi[i]);
				MOGI_members(LOG_MEMBER);
				// check each coordinate element.
				assert(a.cornerA >= mohd->cornerA);
				assert(a.cornerB <= mohd->cornerB);

				// WMO root files are all named world\wmo\<path>.wmo
				// each MOGI in a WMO root file refers to a WMO group file, named on this pattern:
				// world\wmo\<path>_<%03i>.wmo, where i is the index in this for loop.
				// they also refer to minimap textures named like, via md5translate:
				// wmo\<path>_<%03i>_<%02i>_<%02i>.blp, where the other two ints are zero-based x/y indices
				// used to combine the textures into one bitmap that will fit into the box described by
				// the A/B corners of the MOGI.
				// like all world coordinates, north is negative x and west is negative y.
				// minimap textures' origin (0,0) is their northwest corner.
			}
		}
	}
	exit(0);
}

void dumpWDT() {
	MPQFile wdt("World\\Maps\\OrgrimmarInstance\\OrgrimmarInstance.wdt");
	writeFile("output/OrgrimmarInstance.wdt", wdt.getBuffer(), wdt.getSize());

	ChunkedFile cf(wdt.getBuffer(), wdt.getSize());
	bool hasMVER = false;
	while(Chunk c = cf.getChunk()) {
		LOG("chunk %c%c%c%c: %i bytes.\n", c.id[3], c.id[2], c.id[1], c.id[0], c.size);
		if(memcmp(c.id, "REVM", 4) == 0) {
			assert(!hasMVER);
			uint32_t version = *(uint32_t*)c.data;
			LOG("MVER: %i\n", version);
			assert(version == 18);	// vanilla
			hasMVER = true;
			continue;
		}
		assert(hasMVER);
		if(memcmp(c.id, "OMWM", 4) == 0) {
			dumpWMO(c.data);
		}
	}
}
