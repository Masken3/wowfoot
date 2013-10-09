#define __STDC_FORMAT_MACROS
#include "wowVersion.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbcfile.h"
#include "dbc.h"
#include "dbcItemSubClass.h"
#include "util/criticalSection.h"

static CriticalSectionLoadGuard sCS;
ItemSubClasses gItemSubClasses;

// we need to keep this object alive, so the string table remains valid.
static DBCFile sDbc("DBFilesClient\\ItemSubClass.dbc");

void ItemSubClasses::load() {
	LOCK_AND_LOAD;
	DBC::load();

	printf("Opening ItemSubClass.dbc...\n");
	bool res = sDbc.open();
	assert(res);
	printf("Extracting %" PRIuPTR " itemsubclasses...\n", sDbc.getRecordCount());
	int i=-1;
	for(DBCFile::Iterator itr = sDbc.begin(); itr != sDbc.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int itemClass = r.getInt(0);
		int subClass = r.getInt(1);
		ItemSubClass s;
		s.hands = r.getInt(9);
		s.name = r.getString(10);
#if CONFIG_WOW_VERSION > 20000
#define STRING_SIZE 16
#else
#define STRING_SIZE 8
#endif
#if CONFIG_WOW_VERSION > 30000
		s.plural = r.getString(11 + STRING_SIZE);
#else
		s.plural = "";
#endif
		//printf("subClass %i/%i: %i %s %s\n",
			//itemClass, subClass, s.hands, s.name, s.plural);

		if(itemClass == i+1) {
			i++;
			m.push_back(vector<ItemSubClass>());
		}
		if(i < 0)
			continue;
		assert(itemClass == i);
		if(subClass >= (int)m[i].size())
			m[i].resize(subClass+1);
		m[i][subClass] = s;
	}
}
