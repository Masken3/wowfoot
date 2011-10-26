#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbcfile.h"
#include "dbc.h"
#include "dbcSpell.h"

static bool sLoaded = false;
Spells gSpells;

// we need to keep this object alive, so the string table remains valid.
static DBCFile sDbc("DBFilesClient\\Spell.dbc");

void Spells::load() {
	if(sLoaded)
		return;
	sLoaded = true;
	DBC::load();

	printf("Opening Spell.dbc...\n");
	bool res = sDbc.open();
	assert(res);
	printf("Extracting %"PRIuPTR" spells...\n", sDbc.getRecordCount());
	for(DBCFile::Iterator itr = sDbc.begin(); itr != sDbc.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int id = r.getInt(0);
		Spell s;
		s.name = r.getString(136);
		s.rank = r.getString(153);
		s.description = r.getString(170);
		s.toolTip = r.getString(187);
		insert(pair<int, Spell>(id, s));
	}
}
