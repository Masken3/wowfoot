#include "pageContext.h"
#include "chtmlUtil.h"
#include "dbcWorldMapArea.h"
#include "dbcArea.h"
#include "tabTable.h"

#include <vector>

class searchChtml : public tabsChtml, public PageContext {
public:
	searchChtml();
	void title(ostream&);
	int runPage(ostream&);
	void getResponse2(const char* urlPart, DllResponseData* drd, ostream& os);

	const char* urlPart;
};
