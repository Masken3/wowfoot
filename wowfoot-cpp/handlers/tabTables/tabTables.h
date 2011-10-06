#ifndef TAB_TABLES_H
#define TAB_TABLES_H

#include "dllHelpers.h"
#include "chtmlBase.h"

#include <vector>
#include <unordered_map>

struct Column {
	int rowId;
	string title;
	bool noEscape;
	bool hasLink;
	int linkId;
	string linkTarget;
};

enum RowId {
	ENTRY,
};

struct Row : public unordered_map<int, string> {
};

struct Table {
	int id;
	string title;
	vector<Row> array;
	vector<Column> columns;
};

class VISIBLE tabTablesChtml : public ChtmlContext {
public:
	int run(ostream& stream);

	vector<Table> mTables;
};

#endif	//TAB_TABLES_H
