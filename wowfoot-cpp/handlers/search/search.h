#include "chtmlUtil.h"
#include "chtmlBase.h"
#include "dbcWorldMapArea.h"
#include "dbcArea.h"
#include "tabTable.h"

#include <vector>

class searchChtml : public tabsChtml, public PageContext {
public:
	int run(ostream& stream);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* urlPart;
};
