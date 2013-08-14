#include <stdio.h>
#include "spawnPoints.h"
#include "util/exception.h"
#include "areaMap.h"
#include "dbcArea.h"
#include "dbcWorldMapArea.h"

static Coord2D percentagesInZone(int zone, float x, float y);

spawnPointsChtml::spawnPointsChtml() {
	mSpawnCount = 0;
}

void spawnPointsPrepare() {
	gAreaMap.load();
	gWorldMapAreas.load();
	gAreaTable.load();
}

void spawnPointsChtml::prepare() {
	spawnPointsPrepare();
	mSpawnCount = 0;
}

const char* spawnPointsChtml::typeString(Type t) {
	switch(t) {
	case eYellow: return "Yellow";
	case eRed: return "Red";
	case eBlue: return "Blue";
	case eGreen: return "Green";
	default: assert(false);
	}
}

void spawnPointsChtml::addSpawn(int map, float x, float y, Type t) {
	mSpawnCount++;
	int zoneId = zoneFromCoords(map, x, y);
	//printf("zone %i, map %i, %f, %f\n", zoneId, map, x, y);
	SpawnCoord c = { percentagesInZone(zoneId, x, y), t};
	unordered_map<int, Zone>::iterator itr = mZones.find(zoneId);
	Zone* zone = NULL;
	if(itr == mZones.end()) {	// first point in this zone
		const Area* a = gAreaTable.find(zoneId);
		if(a) {
			zone = &mZones[zoneId];
			zone->name = a->name;
		} else {
			printf("spawnPoint in unknown zone %i (map %i, %f, %f)\n",
				zoneId, map, x, y);
		}
	} else {
		zone = &itr->second;
	}
	if(zone != NULL) {
		zone->coords.push_back(c);
		mMainArea = zoneId;
	}
}

void spawnPointsChtml::addSpawns(Spawns::IdPair spawns, Type t) {
	for(; spawns.first != spawns.second; ++spawns.first) {
		const Spawn& s(*spawns.first->second);
		addSpawn(s.map, s.position_x, s.position_y, t);
	}
}

int zoneFromCoords(int map, float x, float y) {
	// find areaId

	// compute grid and cell coordinates
	int globalCellX = (int)((MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS/2) - (y/SIZE_OF_GRID_CELL));
	int globalCellY = (int)((MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS/2) - (x/SIZE_OF_GRID_CELL));
	int gridX = globalCellX / MAX_NUMBER_OF_CELLS;
	int gridY = globalCellY / MAX_NUMBER_OF_CELLS;
	int cellX = globalCellX % MAX_NUMBER_OF_CELLS;
	int cellY = globalCellY % MAX_NUMBER_OF_CELLS;

	//puts "Grid: [#{gridX}][#{gridY}]"
	//puts "Cell: [#{cellX}][#{cellY}]"
	//puts "Pixel: [#{gridX*MAX_NUMBER_OF_CELLS + cellX}][#{gridY*MAX_NUMBER_OF_CELLS + cellY}]"
	int** const* ap = gAreaMap.find(map);
	if(!ap) {
		printf("Invalid mapId(%i) with coords %f x %f\n", map, x, y);
		return -1;
	}
	int** const area(*ap);
	int* grid = area[gridY * MAX_NUMBER_OF_GRIDS + gridX];
	if(!grid)
		return -1;
	int areaId = grid[cellY + cellX * MAX_NUMBER_OF_CELLS];
	EASSERT(areaId >= 0);

	// find zoneId
	int zoneId = areaId;
	do {
		const Area* a = gAreaTable.find(zoneId);
		if(!a) {
			printf("Invalid areaId(%i) for coords %i, %f x %f\n", zoneId, map, x, y);
			return zoneId;
		}
		if(a->parent != 0)
			zoneId = a->parent;
		else
			break;
	} while(true);
	return zoneId;
}

static Coord2D percentagesInZone(int zone, float x, float y) {
	Coord2D p;
	const WorldMapArea* wmap = gWorldMapAreas.find(zone);
	// zones without map images (such as pre-WotLK dungeons).
	// todo: use minimaps and/or 4.0.1 maps.
	if(!wmap)
		return p;
	const WorldMapArea& wma(*wmap);
	float width = wma.a.x - wma.b.x;
	p.x = 1.0 - ((x - wma.b.x) / width);
	float height = wma.a.y - wma.b.y;
	p.y = 1.0 - ((y - wma.b.y) / height);
	return p;
}
