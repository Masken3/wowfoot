#ifndef TDB_RAW_H
#define TDB_RAW_H

#include "dllHelpers.h"

enum ColumnDataType {
	CDT_INT, CDT_STRING, CDT_FLOAT,
};

struct ColumnFormat {
	ColumnDataType type;
	const char* name;
	int offset;
};

// Called before each row has been fetched.
// Returns a pointer to a struct matching the ColumnFormat array.
typedef void* (*TableFetchMap)(int entry);

void fetchTable(const char* tableName, const ColumnFormat*, size_t nCol,
	TableFetchMap) VISIBLE;


// Called after each row has been fetched.
typedef void (*TableFetchSet)();

void fetchTable(const char* tableName, const ColumnFormat*, size_t nCol,
	TableFetchSet, void* dst) VISIBLE;

#endif	//TDB_RAW_H
