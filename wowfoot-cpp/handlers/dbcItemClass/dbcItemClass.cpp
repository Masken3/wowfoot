#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbcfile.h"
#include "dbc.h"
#include "dbcItemClass.h"

static bool sLoaded = false;
ItemClasses gItemClasses;

// we need to keep this object alive, so the string table remains valid.
static DBCFile sDbc("DBFilesClient\\ItemClass.dbc");

void ItemClasses::load() {
	if(sLoaded)
		return;
	sLoaded = true;
	DBC::load();

	printf("Opening ItemClass.dbc...\n");
	bool res = sDbc.open();
	assert(res);
	printf("Extracting %"PRIuPTR" itemclasses...\n", sDbc.getRecordCount());
	m.resize(sDbc.getRecordCount());
	int i=0;
	for(DBCFile::Iterator itr = sDbc.begin(); itr != sDbc.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		ItemClass s;
		s.name = r.getString(3);

		assert(i == id);
		m[i] = s;
		i++;
	}
}
