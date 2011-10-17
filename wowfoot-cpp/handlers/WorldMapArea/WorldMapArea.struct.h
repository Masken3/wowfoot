#ifndef WORLD_MAP_AREA_STRUCT_H
#define WORLD_MAP_AREA_STRUCT_H

#include "coord2d.h"

struct WorldMapArea {
	int map;
	const char* name;
	Coord2D a, b;
};

#endif	//WORLD_MAP_AREA_STRUCT_H
