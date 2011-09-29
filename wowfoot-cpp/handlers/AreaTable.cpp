#include "AreaTable.h"

class AT : public AreaTable {
public:
void load() {
#include "../../wowfoot-ex/output/AreaTable.inl"
}
};

static bool sLoaded;
static AT sAT;
const AreaTable& gAreaTable(sAT);

void AreaTable_ensureLoad() {
	if(sLoaded)
		return;
	sAT.load();
	sLoaded = true;
}
