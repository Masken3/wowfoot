#ifndef TAB_TABLE_H
#define TAB_TABLE_H

#include "dllHelpers.h"
#include "chtmlBase.h"
#include "tabs.h"

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

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

class VISIBLE tabTableChtml : public Tab {
public:
	int run(ostream& stream);

	vector<Row> array;
	vector<Column> columns;
};

#endif	//TAB_TABLE_H
