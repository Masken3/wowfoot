#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbcfile.h"
#include "dbc.h"
#include "dbcItemSubClass.h"
#include "util/criticalSection.h"

static CriticalSection sCS;
static bool sLoaded = false;
ItemSubClasses gItemSubClasses;

// we need to keep this object alive, so the string table remains valid.
static DBCFile sDbc("DBFilesClient\\ItemSubClass.dbc");

void ItemSubClasses::load() {
	LOCK(sCS);
	if(sLoaded)
		return;
	sLoaded = true;
	DBC::load();

	printf("Opening ItemSubClass.dbc...\n");
	bool res = sDbc.open();
	assert(res);
	printf("Extracting %"PRIuPTR" itemsubclasses...\n", sDbc.getRecordCount());
	int i=-1;
	for(DBCFile::Iterator itr = sDbc.begin(); itr != sDbc.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int itemClass = r.getInt(0);
		int subClass = r.getInt(1);
		ItemSubClass s;
		s.hands = r.getInt(9);
		s.name = r.getString(10);
		s.plural = r.getString(27);
//		printf("subClass %i/%i: %i %s %s\n",
//			itemClass, subClass, s.hands, s.name, s.plural);

		if(itemClass == i+1) {
			i++;
			m.push_back(vector<ItemSubClass>());
		}
		assert(itemClass == i);
		if(subClass >= (int)m[i].size())
			m[i].resize(subClass+1);
		m[i][subClass] = s;
	}
}
