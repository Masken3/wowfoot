#include "chtmlUtil.h"
#include "chtmlBase.h"
#include "WorldMapArea.h"
#include "AreaTable.h"
#include "tabTables.h"

#include <vector>

class searchChtml : public tabTablesChtml {
public:
	int run(ostream& stream);
	const char* urlPart;
};
