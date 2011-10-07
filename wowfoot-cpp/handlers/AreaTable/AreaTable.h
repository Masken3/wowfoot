#ifndef AREA_TABLE_H
#define AREA_TABLE_H

#include "ConstMap.h"
#include "dllHelpers.h"

struct Area {
	int map;
	int parent;
	int level;
	const char* name;	//encoding: utf-8
};
typedef ConstMap<int, Area> AreaTable;
const extern AreaTable& gAreaTable VISIBLE;
void AreaTable_ensureLoad() VISIBLE;

#endif	//AREA_TABLE_H
