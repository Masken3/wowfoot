#include "chtmlUtil.h"
#include "chtmlBase.h"
#include "WorldMapArea.h"
#include "AreaTable.h"

class searchChtml : public ChtmlContext {
public:
	int run(ostream& stream);
	const char* urlPart;
};
