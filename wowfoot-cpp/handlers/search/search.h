#include "chtmlUtil.h"
#include "chtmlBase.h"
#include "WorldMapArea.h"
#include "AreaTable.h"
#include "tabTable.h"

#include <vector>

class searchChtml : public tabsChtml {
public:
	int run(ostream& stream);
	const char* urlPart;
};
