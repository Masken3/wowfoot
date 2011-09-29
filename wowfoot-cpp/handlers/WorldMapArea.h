#ifndef WORLD_MAP_AREA_H
#define WORLD_MAP_AREA_H

#include "chtmlUtil.h"
#include "ConstMap.h"
#include "coord2d.h"

struct WorldMapArea {
	int map;
	string name;
	Coord2D a, b;
};
typedef ConstMap<int, WorldMapArea> WorldMapAreas;
const extern WorldMapAreas& gWorldMapAreas;
void WorldMapAreas_ensureLoad();

#endif	//WORLD_MAP_AREA_H
