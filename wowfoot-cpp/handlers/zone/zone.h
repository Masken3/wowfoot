#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "WorldMapArea.h"
#include "AreaTable.h"
#include "mapSize.h"

class zoneChtml : public ChtmlContext {
public:
	int run(ostream& stream);
	const char* urlPart;
};