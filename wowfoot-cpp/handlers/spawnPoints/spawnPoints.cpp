#include "spawnPoints.h"
#include "util/exception.h"
#include "areaMap.h"

static int zoneFromCoords(int map, float x, float y);
static Coord2D percentagesInZone(int zone, float x, float y);

void spawnPointsChtml::prepare() {
	gAreaMap.load();
	gWorldMapAreas.load();

	mSpawnCount = 0;
	for(; mSpawns.first != mSpawns.second; ++mSpawns.first) {
		const Spawn& s(*mSpawns.first->second);
		mSpawnCount++;
		int zoneId = zoneFromCoords(s.map, s.position_x, s.position_y);
		//printf("zone %i, map %i, %f x %f\n", zoneId, s.map, s.position_x, s.position_y);
		Coord2D c = percentagesInZone(zoneId, s.position_x, s.position_y);
		mZones[zoneId].coords.push_back(c);
		mMainArea = zoneId;
	}
}

static int zoneFromCoords(int map, float x, float y) {
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
	int* grid = gAreaMap[map][gridY * MAX_NUMBER_OF_GRIDS + gridX];
	if(!grid)
		return -1;
	int areaId = grid[cellY + cellX * MAX_NUMBER_OF_CELLS];
	EASSERT(areaId >= 0);

	// find zoneId
	int zoneId = areaId;
	while(gAreaTable[zoneId].parent != 0) {
		zoneId = gAreaTable[zoneId].parent;
	}
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
