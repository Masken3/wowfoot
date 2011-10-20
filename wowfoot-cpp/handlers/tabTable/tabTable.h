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
	enum NoEscapeEnum {
		Escape,
		NoEscape,
	};
#if 0
	// Generic
	Column(int rid, const char* t, NoEscapeEnum e = Escape, const char* lt = NULL, int lid = -1)
	: rowId(rid), title(t), noEscape(e == NoEscape), hasLink(lid >= 0), linkId(lid), linkTarget(lt)
	{}
#endif
	// NoEscape
	Column(int rid, const char* t, NoEscapeEnum e = Escape)
	: rowId(rid), title(t), noEscape(e == NoEscape), hasLink(false), linkId(-1)
	{}
	// Link
	Column(int rid, const char* t, int lid, const char* lt)
	: rowId(rid), title(t), noEscape(false), hasLink(lid >= 0), linkId(lid), linkTarget(lt)
	{}
	int rowId;
	const char* title;
	bool noEscape;
	bool hasLink;
	int linkId;
	const char* linkTarget;
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
