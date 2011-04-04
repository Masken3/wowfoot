#include "config.h"
#include "libs/mpq_libmpq04.h"
#include "libs/dbcfile.h"

int main() {
	printf("Opening locale.mpq...\n");
	MPQArchive locale(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/locale-"WOW_LOCALE".MPQ");

	printf("Opening WorldMapContinent.dbc...\n");
	DBCFile wmc("DBFilesClient\\WorldMapContinent.dbc");
	bool res = wmc.open();
	if(!res)
		return 1;
	printf("Extracting %i continents...\n", wmc.getRecordCount());
	for(DBCFile::Iterator itr = wmc.begin(); itr != wmc.end(); ++itr) {
		const DBCFile::Record& r(*itr);
		int cid = r.getInt(1);
		int mid = r.getInt(2);
		float x1 = r.getFloat(9);
		float y1 = r.getFloat(10);
		float x2 = r.getFloat(11);
		float y2 = r.getFloat(12);
		printf("%i, %i, %g, %g, %g, %g\n", cid, mid, x1, y1, x2, y2);
	}

	printf("Opening Map.dbc...\n");
	DBCFile map("DBFilesClient\\Map.dbc");

	return 0;
}
