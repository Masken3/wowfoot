#include "chtmlBase.h"
#include "chtmlUtil.h"
#include "dbcWorldMapArea.h"
#include "dbcArea.h"
#include "mapSize.h"

class zoneChtml : public ChtmlContext {
public:
	int run(ostream& stream);
	const char* urlPart;
};
