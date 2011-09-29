#include "WorldMapArea.h"

class WMA : public WorldMapAreas {
public:
void load() {
#include "../../wowfoot-ex/output/WorldMapArea.inl"
}
};

static bool sLoaded;
static WMA sWMA;
const WorldMapAreas& gWorldMapAreas(sWMA);

void WorldMapAreas_ensureLoad() {
	if(sLoaded)
		return;
	sWMA.load();
	sLoaded = true;
}
