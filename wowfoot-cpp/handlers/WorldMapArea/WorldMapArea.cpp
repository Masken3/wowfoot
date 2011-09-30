#include "WorldMapArea.data.h"

class WMA : public WorldMapAreas {
public:
	void load() {
		for(size_t i=0; i<gnWMA; i++) {
			insert(pair<int, WorldMapArea>(gWMA[i].id, gWMA[i].a));
		}
	}
};

static bool sLoaded = false;
static WMA sWMA;
const WorldMapAreas& gWorldMapAreas(sWMA);

void WorldMapAreas_ensureLoad() {
	if(sLoaded)
		return;
	sWMA.load();
	sLoaded = true;
}
