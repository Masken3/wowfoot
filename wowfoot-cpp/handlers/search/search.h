#include "chtmlUtil.h"
#include "chtmlBase.h"
#include "WorldMapArea.h"
#include "AreaTable.h"

#include <vector>

class searchChtml : public ChtmlContext {
public:
	int run(ostream& stream);
	const char* urlPart;
	vector<pair<int, Area> > areas;
};
