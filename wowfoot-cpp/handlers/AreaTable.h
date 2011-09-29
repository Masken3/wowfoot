#ifndef AREA_TABLE_H
#define AREA_TABLE_H

#include "chtmlUtil.h"
#include "ConstMap.h"

struct Area {
	int map;
	int parent;
	int level;
	string name;	//encoding: utf-8
};
typedef ConstMap<int, Area> AreaTable;
const extern AreaTable& gAreaTable;
void AreaTable_ensureLoad();

#endif	//AREA_TABLE_H
