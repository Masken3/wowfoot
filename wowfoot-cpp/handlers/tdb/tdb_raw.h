#ifndef TDB_RAW_H
#define TDB_RAW_H

#include "dllHelpers.h"

enum ColumnDataType {
	CDT_INT, CDT_STRING, CDT_FLOAT,
};

struct ColumnFormat {
	ColumnDataType type;
	const char* name;
};

// Called before each row has been fetched.
// Returns a pointer to a struct matching the ColumnFormat array.
typedef void* (*TableFetchCallback)(int entry);

void fetchTable(const char* tableName, const ColumnFormat*, size_t nCol,
	TableFetchCallback) VISIBLE;

#endif	//TDB_RAW_H
