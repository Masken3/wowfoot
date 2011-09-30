#ifndef WORLD_MAP_AREA_H
#define WORLD_MAP_AREA_H

#include "ConstMap.h"
#include "coord2d.h"
#include "dllHelpers.h"

struct WorldMapArea {
	int map;
	string name;
	Coord2D a, b;
};
typedef ConstMap<int, WorldMapArea> WorldMapAreas;
const extern WorldMapAreas& gWorldMapAreas VISIBLE;
void WorldMapAreas_ensureLoad() VISIBLE;

#endif	//WORLD_MAP_AREA_H
