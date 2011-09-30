#include "AreaTable.data.h"

class AT : public AreaTable {
public:
	void load() {
		for(size_t i=0; i<gnAT; i++) {
			insert(pair<int, Area>(gAT[i].id, gAT[i].a));
		}
	}
};

static bool sLoaded = false;
static AT sAT;
const AreaTable& gAreaTable(sAT);

void AreaTable_ensureLoad() {
	if(sLoaded)
		return;
	sAT.load();
	sLoaded = true;
}
