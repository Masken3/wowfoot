#ifndef AREA_TABLE_STRUCT_H
#define AREA_TABLE_STRUCT_H

struct Area {
	int map;
	int parent;
	int level;
	const char* name;	//encoding: utf-8
};

#endif	//AREA_TABLE_STRUCT_H
